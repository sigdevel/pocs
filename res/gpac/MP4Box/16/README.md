e95f3064d846e4606276fff111e0f97df1576a04

https://github.com/gpac/gpac/issues/3236

https://github.com/gpac/gpac/commit/df0c81722847238659a6beb0feab2c1ecd05c020



1. Vulnerability type: 
Heap Buffer Overflow

2. Vendor of the product(s):
GPAC Multimedia Open Source Project

3. Vendor of the product(s) version:
2.4

4. Affected product(s)/code base:
MP4Box (isomedia/drm_sample.c)

5. Impact:
Denial of Service

6. Attack type:
Local

7. Affected component(s):
isomedia/drm_sample.c ; gf_cenc_set_pssh ; L:982 ; (memcpy(dst_buffer, src_data, data_size)) ;

8. Attack vector(s):
Network (AV:N): Processing of specially crafted mp4-files containing malformed PSSH boxes through DASH-segmentation services may trigger heap overflow during -dash operations ;
Local (AV:L): Execution of MP4Box -dash 10000 commands on manipulated mp4-files initiates heap corruption via DRM header parsing routines ;

9. Suggested CVE description:
A heap-buffer-overflow vulnerability occurs in GPAC's gf_cenc_set_pssh function (isomedia/drm_sample.c) during processing of malformed PSSH boxes. When mp4-files containing improperly structured Protection System Specific Headers (e.g., with 34 extra bytes) undergo DASH segmentation, unvalidated data_size parameters in memcpy operations cause 2097152016 bytes to be written into 512-byte heap buffers. This results in memory corruption, application termination, or potential code execution due to insufficient bounds checking during handling of unrecognized protection schemes (type 00000000).

10. Discoverer(s)/Credits info:
Alexander A. Shvedov (@sigdevel)

11. Reference(s) info:
https://github.com/gpac/gpac/issues/3236
https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/16/16_poc.mp4
https://github.com/gpac/gpac/commit/df0c81722847238659a6beb0feab2c1ecd05c020
12. Additional information:
At this point, the vulnerability is public knowledge and affects all versions of GPAC, including the latest version: 2.4. The developer has proposed a patch to fix this issue, and the fix has been added to the main branch of the repository on GitHub.
1. Reporting to the vendor: https://github.com/gpac/gpac/issues/3236
2. PoC: https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/16/16_poc.mp4
3. Patch: https://github.com/gpac/gpac/commit/df0c81722847238659a6beb0feab2c1ecd05c020


