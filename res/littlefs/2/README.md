6cb4e86540eca0d9ba62500a298385c9d863c8be

littlefs (https://github.com/littlefs-project/littlefs), when mounted with `cfg.block_count = 0`:

- v2.8.0 – v2.11.3 (latest, master `6cb4e86`): affected. Confirmed under UBSan/ASan on every release tag: division by zero in `lfs_mount_()`. The `block_count = 0` mode (block count read from the superblock) first appeared in v2.8.0 (commits `be681221` and `9c23329d`, 2023-08/09).
- v2.2.0 – v2.7.1: not affected. They have no `block_count = 0` mode and `lfs_mount()` returns `LFS_ERR_CORRUPT` (-84).
- v2.0.0 – v2.1.4: not affected by this bug. With `block_count = 0` they abort on a configuration assert.
- Not fixed upstream as of 2026-10-18.

littlefs-python (https://github.com/jrast/littlefs-python, PyPI `littlefs-python`):

- v0.19.0 (latest, bundles littlefs v2.11.3): affected and confirmed. `littlefs-python list/extract --block-size 256 <img>` is killed with SIGFPE (exit 136).
- v0.17.1 – v0.18.0 (bundle littlefs v2.11.2): affected according to source analysis, not tested. The CLI `list`/`extract`/`repl` commands mount with `block_count=0` from v0.17.1 on (`_mount_from_context()`).
- v0.9.0 – v0.17.0 (bundle littlefs v2.8.0 – v2.11.2): the bundled library is vulnerable, but the CLI does not use `block_count=0`. Only applications that call the Python API with `block_count=0` are exposed. Not tested.

littlefs-fuse (https://github.com/littlefs-project/littlefs-fuse) v2.7.15: not affected. It always passes a non-zero `block_count`, from `--block_count` or from `ioctl(BLKGETSIZE64)`.