74fecde32cd477ab097f3e6db55a32b259f3313d

https://github.com/gpac/gpac/issues/3240

https://github.com/gpac/gpac/commit/ad3b541b4f38c8f0ef67544509598f8207ea1207


1. Vulnerability type: 
NULL Pointer Dereference

2. Vendor of the product(s):
GPAC Multimedia Open Source Project

3. Vendor of the product(s) version:
2.4

4. Affected product(s)/code base:
MP4Box (filters/mux_isom.c)

5. Impact:
Denial of Service

6. Attack type:
Local

7. Affected component(s):
filters/mux_isom.c ; TrackWriter ; L:6621

8. Attack vector(s):
Processing specially crafted mp4-files containing malformed metadata boxes (ex., mvcC or stsz with extra bytes) using MP4Box commands like -dash or -add triggers the vulnerability

9. Suggested CVE description:
A NULL pointer dereference vulnerability exists in GPAC's MP4Box within the isom_cenc_get_sai_by_saiz_saio function (src/isomedia/drm_sample.c). When processing MP4 files containing malformed DRM metadata or invalid box structures, the application fails to validate pointer references before access. This causes a segmentation fault during operations like DASH segmentation or file muxing, resulting in application termination and DOS.

10. Discoverer(s)/Credits info:
@sigdevel

11. Reference(s) info:
https://github.com/gpac/gpac/issues/3240
https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/17/17_poc.mp4
https://github.com/gpac/gpac/commit/ad3b541b4f38c8f0ef67544509598f8207ea1207

12. Additional information:
At this point, the vulnerability is public knowledge and affects all versions of GPAC, including the latest version: 2.4. The developer has proposed a patch to fix this issue, and the fix has been added to the main branch of the repository on GitHub.
1. Reporting to the vendor: https://github.com/gpac/gpac/issues/3240
2. PoC: https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/17/17_poc.mp4
3. Patch: https://github.com/gpac/gpac/commit/ad3b541b4f38c8f0ef67544509598f8207ea1207

