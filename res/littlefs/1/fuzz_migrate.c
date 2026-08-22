// fuzz_migrate.c
// Harness for the littlefs v1 -> v2 migration parser (lfs_migrate) in
// lfs.c, built with -DLFS_MIGRATE.
//
// The fuzz input is treated as the raw contents of a small RAM block
// device, formatted (or not) as a littlefs v1 filesystem. lfs_migrate()
// is called on it, exercising the legacy lfs1_* superblock/directory
// parsing code. If migration succeeds, the harness mounts the resulting
// v2 filesystem and runs the same traversal as fuzz_mount, exercising the
// v2 metadata parser on attacker-influenced (migrated) data as well.
//
// Works with file input (argv[1]) or stdin (AFL without @@).
// Provides LLVMFuzzerTestOneInput entrypoint + main() for AFL/vanilla.

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "lfs.h"

// Small, fixed geometry. Kept small so a single mutated input can plausibly
// reach the on-disk v1 superblock/directory layout while still being cheap
// to run millions of times.
#define BLOCK_SIZE      512
#define BLOCK_COUNT     32
#define DISK_SIZE       (BLOCK_SIZE * BLOCK_COUNT)
#define READ_SIZE       16
#define PROG_SIZE       16
#define CACHE_SIZE      64
#define LOOKAHEAD_SIZE  16

#define PATH_MAX_LEN    512

// Bounds the amount of work done per input so that corrupted images that
// describe cycles (e.g. a directory entry pointing back at an ancestor)
// cannot turn a single execution into an infinite loop.
#define OP_BUDGET       4096
#define MAX_DEPTH       8

static uint8_t disk[DISK_SIZE];

static int bd_read(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size) {
    (void)c;
    if (block >= BLOCK_COUNT || off + size > BLOCK_SIZE) {
        return LFS_ERR_IO;
    }
    memcpy(buffer, &disk[block * BLOCK_SIZE + off], size);
    return 0;
}

static int bd_prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    (void)c;
    if (block >= BLOCK_COUNT || off + size > BLOCK_SIZE) {
        return LFS_ERR_IO;
    }
    memcpy(&disk[block * BLOCK_SIZE + off], buffer, size);
    return 0;
}

static int bd_erase(const struct lfs_config *c, lfs_block_t block) {
    (void)c;
    if (block >= BLOCK_COUNT) {
        return LFS_ERR_IO;
    }
    memset(&disk[block * BLOCK_SIZE], 0xff, BLOCK_SIZE);
    return 0;
}

static int bd_sync(const struct lfs_config *c) {
    (void)c;
    return 0;
}

// --- v1 directory CRC fixup -------------------------------------------
//
// lfs1_dir_fetch() rejects a directory block unless a CRC32 computed over
// bytes [0, size) (size taken from the block's own header) comes out to
// zero. Random mutations satisfy a 32-bit CRC essentially never, so without
// this fixup AFL bounces off the very first parsing step for almost every
// input and coverage plateaus around ~19% (see fuzz/migrate notes).
//
// CRC32 is GF(2)-linear: crc_apply4(crc, x) = M*x XOR N*crc for fixed linear
// maps M, N independent of crc/x. So M (and its inverse) can be computed
// once, and "find x such that crc_apply4(crc, x) == 0" becomes a handful of
// XOR/popcount ops instead of a fresh 32x32 Gaussian elimination per call.
static void put_le32_val(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >>  0);
    p[1] = (uint8_t)(v >>  8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static uint32_t get_le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
            ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint32_t crc_apply4(uint32_t crc, uint32_t x) {
    uint8_t bytes[4];
    put_le32_val(bytes, x);
    return lfs_crc(crc, bytes, sizeof(bytes));
}

static uint32_t g_crc_inv_row[32];
static int g_crc_inv_ready = 0;

// One-time computation of M^-1, represented as 32 row-masks: solving
// M*x = target reduces to x_c = parity(g_crc_inv_row[c] & target).
static void crc_fix_init(void) {
    uint32_t f0 = crc_apply4(0, 0);
    uint32_t basis[32];
    for (int i = 0; i < 32; i++) {
        basis[i] = crc_apply4(0, (uint32_t)1u << i) ^ f0;
    }

    uint32_t row[32];
    uint32_t id[32];
    for (int r = 0; r < 32; r++) {
        uint32_t v = 0;
        for (int c = 0; c < 32; c++) {
            if ((basis[c] >> r) & 1u) {
                v |= (1u << c);
            }
        }
        row[r] = v;
        id[r] = (uint32_t)1u << r;
    }

    // Gauss-Jordan elimination of [M | I] -> [I | M^-1].
    for (int c = 0; c < 32; c++) {
        int sel = -1;
        for (int i = c; i < 32; i++) {
            if ((row[i] >> c) & 1u) {
                sel = i;
                break;
            }
        }
        if (sel < 0) {
            continue;
        }

        uint32_t t;
        t = row[sel]; row[sel] = row[c]; row[c] = t;
        t = id[sel];  id[sel]  = id[c];  id[c]  = t;

        for (int i = 0; i < 32; i++) {
            if (i != c && ((row[i] >> c) & 1u)) {
                row[i] ^= row[c];
                id[i] ^= id[c];
            }
        }
    }

    for (int c = 0; c < 32; c++) {
        g_crc_inv_row[c] = id[c];
    }
    g_crc_inv_ready = 1;
}

// Returns x such that lfs_crc(crc, le32(x), 4) == 0.
static uint32_t crc_fix(uint32_t crc) {
    if (!g_crc_inv_ready) {
        crc_fix_init();
    }

    uint32_t target = crc_apply4(crc, 0);
    uint32_t x = 0;
    for (int c = 0; c < 32; c++) {
        if (__builtin_popcount(g_crc_inv_row[c] & target) & 1u) {
            x |= (uint32_t)1u << c;
        }
    }
    return x;
}

// Patches the trailing 4-byte CRC of every block whose header `size` field
// (bytes [4,8), masked with 0x7fffffff) falls in lfs1_dir_fetch's accepted
// range [sizeof(lfs1_disk_dir)+4, block_size]. This lets AFL's mutations
// freely define directory revs/tails/entries while keeping the block
// "CRC-valid", exposing lfs_migrate_()'s entry-copy and v2-recreation logic.
static void fixup_v1_dir_crcs(uint8_t *d) {
    for (int b = 0; b < BLOCK_COUNT; b++) {
        uint8_t *blk = &d[b * BLOCK_SIZE];
        uint32_t size = get_le32(&blk[4]) & 0x7fffffff;
        if (size < 20 || size > BLOCK_SIZE) {
            continue;
        }

        uint32_t crc = lfs_crc(0xffffffff, blk, 16);
        crc = lfs_crc(crc, &blk[16], size - 20);
        put_le32_val(&blk[size - 4], crc_fix(crc));
    }
}

static unsigned op_budget;

// Reads a file to completion (or until the op budget runs out).
static void read_lfs_file(lfs_t *lfs, const char *path) {
    lfs_file_t file;
    if (lfs_file_open(lfs, &file, path, LFS_O_RDONLY) < 0) {
        return;
    }

    uint8_t buf[256];
    while (op_budget > 0) {
        op_budget--;
        lfs_ssize_t r = lfs_file_read(lfs, &file, buf, sizeof(buf));
        if (r <= 0) {
            break;
        }
    }

    lfs_file_close(lfs, &file);
}

// Recursively walks a directory, reading every file it finds.
static void walk(lfs_t *lfs, char *path, size_t len, int depth) {
    if (depth > MAX_DEPTH || op_budget == 0) {
        return;
    }

    lfs_dir_t dir;
    if (lfs_dir_open(lfs, &dir, path) < 0) {
        return;
    }

    struct lfs_info info;
    while (op_budget > 0) {
        op_budget--;
        int res = lfs_dir_read(lfs, &dir, &info);
        if (res <= 0) {
            break;
        }

        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0) {
            continue;
        }

        size_t name_len = strlen(info.name);
        size_t sep = (len > 0 && path[len - 1] != '/') ? 1 : 0;
        if (len + sep + name_len + 1 > PATH_MAX_LEN) {
            continue;
        }

        if (sep) {
            path[len] = '/';
        }
        memcpy(path + len + sep, info.name, name_len);
        size_t new_len = len + sep + name_len;
        path[new_len] = '\0';

        if (info.type == LFS_TYPE_DIR) {
            walk(lfs, path, new_len, depth + 1);
        } else {
            read_lfs_file(lfs, path);
        }

        path[len] = '\0';
    }

    lfs_dir_close(lfs, &dir);
}

static int traverse_cb(void *data, lfs_block_t block) {
    (void)data;
    (void)block;
    if (op_budget == 0) {
        return LFS_ERR_CORRUPT;
    }
    op_budget--;
    return 0;
}

static void run_one_input(const uint8_t *data, size_t size) {
    if (!data) {
        return;
    }

    memset(disk, 0, sizeof(disk));
    size_t n = (size < DISK_SIZE) ? size : DISK_SIZE;
    memcpy(disk, data, n);
    fixup_v1_dir_crcs(disk);

    struct lfs_config cfg = {0};
    cfg.read = bd_read;
    cfg.prog = bd_prog;
    cfg.erase = bd_erase;
    cfg.sync = bd_sync;
    cfg.read_size = READ_SIZE;
    cfg.prog_size = PROG_SIZE;
    cfg.block_size = BLOCK_SIZE;
    cfg.block_count = BLOCK_COUNT;
    cfg.cache_size = CACHE_SIZE;
    cfg.lookahead_size = LOOKAHEAD_SIZE;
    cfg.block_cycles = 500;

    lfs_t lfs;

    // Exercise the v1 superblock/directory parser. lfs_migrate() clobbers
    // the lfs_t and leaves the filesystem unmounted on return.
    if (lfs_migrate(&lfs, &cfg) != 0) {
        return;
    }

    // If migration succeeded, the disk image now contains a v2 filesystem
    // derived from attacker-controlled v1 data. Mount and traverse it the
    // same way fuzz_mount does to exercise the v2 metadata parser too.
    if (lfs_mount(&lfs, &cfg) != 0) {
        return;
    }

    op_budget = OP_BUDGET;

    char path[PATH_MAX_LEN];
    path[0] = '/';
    path[1] = '\0';
    walk(&lfs, path, 1, 0);

    if (op_budget > 0) {
        (void)lfs_fs_traverse(&lfs, traverse_cb, NULL);
    }

    struct lfs_fsinfo fsinfo;
    (void)lfs_fs_stat(&lfs, &fsinfo);
    (void)lfs_fs_size(&lfs);

    lfs_unmount(&lfs);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    run_one_input(data, size);
    return 0;
}

// -------------------------
// Standalone main (vanilla + AFL)
// -------------------------
#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION) || defined(AFL_BUILD)

static int read_all_stdin(uint8_t **out, size_t *out_len)
{
	size_t cap = 1u << 16;
	size_t len = 0;
	uint8_t *buf = (uint8_t *)malloc(cap);
	if (!buf) return -1;

	for (;;) {
		size_t want = cap - len;
		if (want == 0) {
			cap *= 2;
			uint8_t *nb = (uint8_t *)realloc(buf, cap);
			if (!nb) { free(buf); return -1; }
			buf = nb;
			want = cap - len;
		}
		size_t rd = fread(buf + len, 1, want, stdin);
		len += rd;
		if (rd == 0) break;
	}

	*out = buf;
	*out_len = len;
	return 0;
}

static int read_file(const char *path, uint8_t **out, size_t *out_len)
{
	FILE *fp = fopen(path, "rb");
	if (!fp) return -1;

	if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return -1; }
	long sz = ftell(fp);
	if (sz < 0) { fclose(fp); return -1; }
	if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return -1; }

	size_t n = (size_t)sz;
	uint8_t *buf = (uint8_t *)malloc(n ? n : 1);
	if (!buf) { fclose(fp); return -1; }

	size_t rd = fread(buf, 1, n, fp);
	fclose(fp);
	if (rd != n) { free(buf); return -1; }

	*out = buf;
	*out_len = n;
	return 0;
}

// Persistent mode: when built with afl-clang-fast, __AFL_LOOP() runs many
// inputs inside a single forked process, avoiding per-input fork/exec
// overhead (typically a 10-20x speedup). Falls back to a single
// file/stdin run for vanilla builds or non-AFL execution.
#ifdef __AFL_FUZZ_INIT
__AFL_FUZZ_INIT();
#endif

int main(int argc, char **argv)
{
#ifdef __AFL_FUZZ_TESTCASE_LEN
	(void)argc;
	(void)argv;

#ifdef __AFL_HAVE_MANUAL_CONTROL
	__AFL_INIT();
#endif

	uint8_t *afl_buf = __AFL_FUZZ_TESTCASE_BUF;
	while (__AFL_LOOP(10000)) {
		size_t afl_len = (size_t)__AFL_FUZZ_TESTCASE_LEN;
		run_one_input(afl_buf, afl_len);
	}
	return 0;
#else
	uint8_t *buf = NULL;
	size_t len = 0;

	// argv[1] path OR stdin (AFL without @@)
	if (argc >= 2) {
		if (read_file(argv[1], &buf, &len) != 0)
			return 0;
	} else {
		if (read_all_stdin(&buf, &len) != 0)
			return 0;
	}

	(void)LLVMFuzzerTestOneInput(buf, len);
	free(buf);
	return 0;
#endif
}
#endif
