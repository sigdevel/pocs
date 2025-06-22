f87b30611380e4dcd03cd4dd9ac553c0ec336826

https://github.com/gpac/gpac/issues/3195

https://github.com/gpac/gpac/commit/e38d24b7e3cbdc24e70f0437bf390ac3f2080b52


1. Vulnerability type: 
NULL Pointer Dereference

2. Vendor of the product(s):
GPAC Multimedia Open Source Project

3. Vendor of the product(s) version:
2.4

4. Affected product(s)/code base:
MP4Box (isomedia/isom_write.c)

5. Impact:
Code Execution, Denial of Service

6. Attack type:
Local

7. Affected component(s):
isomedia/isom_write.c ; gf_isom_copy_sample_info ; L:8164

8. Attack vector(s):
-Network (AV:N): Processing of crafted MP4 files with corrupted SAI metadata (e.g., sai_samples=1836253208) in media handling services using MP4Box may trigger segmentation faults during file splitting operations (-split-size) ;
-Local (AV:L): Execution of MP4Box commands (e.g., -add [file] -new /dev/null) on specially structured media files may cause NULL pointer dereference due to unvalidated pointers following memory allocation failures ;

9. Suggested CVE description:
A NULL pointer dereference vulnerability exists in the gf_isom_copy_sample_info function of GPAC's MP4Box (isomedia/isom_write.c). Processing MP4 files with malformed Sample Auxiliary Information (SAI) metadata causes failure to validate pointers after handling invalid sai_samples values. This allows crafted files to crash MP4Box during operations like -add or -split-size, resulting in Denial of Service. The flaw stems from missing NULL checks when copying sample information (memcpy(dst_track->sample_info, ...)), triggering segfaults.

10. Discoverer(s)/Credits info:
Alexander A. Shvedov (@sigdevel)

11. Reference(s) info:
https://github.com/gpac/gpac/issues/3195
https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/13/13_poc.mp4
https://github.com/gpac/gpac/commit/e38d24b7e3cbdc24e70f0437bf390ac3f2080b52

12. Additional information:
At this point, the vulnerability is public knowledge and affects all versions of GPAC, including the latest version: 2.4. The developer has proposed a patch to fix this issue, and the fix has been added to the main branch of the repository on GitHub.
1. Reporting to the vendor: https://github.com/gpac/gpac/issues/3195
2. PoC: https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/13/13_poc.mp4
3. Patch: https://github.com/gpac/gpac/commit/e38d24b7e3cbdc24e70f0437bf390ac3f2080b52


