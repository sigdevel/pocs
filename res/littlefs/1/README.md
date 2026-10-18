6cb4e86540eca0d9ba62500a298385c9d863c8be

https://github.com/superfly/litefs/issues/452

littlefs (https://github.com/littlefs-project/littlefs), built with `LFS_MIGRATE`:

- v2.2.0 – v2.11.3 (latest, master `6cb4e86`): affected. Confirmed under ASan on every release tag: stack-buffer-overflow READ in `lfs_dir_traverse()` (`lfs.c:752` in v2.2.x … `lfs.c:1059` in v2.11.3).
- v2.0.3 – v2.1.4: not confirmed. The PoC fails earlier on an `LFS_ASSERT` in the block device (`block < lfs->cfg->block_count`, SIGABRT).
- v2.0.0 – v2.0.2: not affected by this PoC (no crash).
- Not fixed upstream as of 2026-10-18.

littlefs-fuse (https://github.com/littlefs-project/littlefs-fuse), which always builds with `-DLFS_MIGRATE` and is reached via `lfs --migrate`:

- v2.2.0 – v2.7.15 (latest, `423fc82`; bundles littlefs 2.2 – 2.11.3): affected. Checked with the ASan build on v2.2.0, v2.4.0, v2.6.1, v2.7.0 and v2.7.15. The stock build of v2.7.15 crashes with SIGSEGV or SIGABRT in about 80% of runs.
- v2.0.0 (bundles littlefs 2.0): not confirmed (SIGABRT on the same block-device assert).

littlefs-python (https://github.com/jrast/littlefs-python) 0.19.0: not affected. The `lfs_migrate` symbol is absent from the bundled extension module.
