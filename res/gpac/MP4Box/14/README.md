f87b30611380e4dcd03cd4dd9ac553c0ec336826

https://github.com/gpac/gpac/issues/3196

https://github.com/gpac/gpac/commit/cea49f684dbc4d53ecd6c76a9623838802a68d88



1. Vulnerability type: 
Divide by Zero

2. Vendor of the product(s):
GPAC Multimedia Open Source Project

3. Vendor of the product(s) version:
2.4

4. Affected product(s)/code base:
MP4Box (filters/dmx_avi.c)

5. Impact:
Denial of Service

6. Attack type:
Local

7. Affected component(s):
filters/dmx_avi.c ; avidmx_process ; L:639

8. Attack vector(s):
Processing of specially crafted files (ex., AVI-type containing 0/256 frame declarations) through the -dash command triggers division-by-zero during bitrate computation, resulting in application termination

9. Suggested CVE description:
A division-by-zero vulnerability exists in GPAC's MP4Box within the avidmx_process function (filters/dmx_avi.c). During processing of files containing invalid frame metadata (notably num_frames=0), missing validation of frame count values before division operations causes floating-point exceptions when executing DASH segmentation commands (-dash). This leads to denial-of-service conditions through application crashes. The vulnerability originates from unverified divisor usage at line 639 where total_bits / num_frames is calculated without ensuring num_frames > 0.

10. Discoverer(s)/Credits info:
Alexander A. Shvedov (@sigdevel)

11. Reference(s) info:
https://github.com/gpac/gpac/issues/3196
https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/14/14_poc.mp4
https://github.com/gpac/gpac/commit/cea49f684dbc4d53ecd6c76a9623838802a68d88

12. Additional information:
At this point, the vulnerability is public knowledge and affects all versions of GPAC, including the latest version: 2.4. The developer has proposed a patch to fix this issue, and the fix has been added to the main branch of the repository on GitHub.
1. Reporting to the vendor: https://github.com/gpac/gpac/issues/3196
2. PoC: https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/14/14_poc.mp4
3. Patch: https://github.com/gpac/gpac/commit/cea49f684dbc4d53ecd6c76a9623838802a68d88


