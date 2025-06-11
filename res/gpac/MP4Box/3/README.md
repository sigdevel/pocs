8a0d5b43c242fe4befb88530e4c9afef37114161

https://github.com/gpac/gpac/issues/3146

https://github.com/makesoftwaresafe/gpac/commit/d091c7e92ef0b6497b808e243501f500135f69c4

1. Vulnerability type: Buffer Overflow

2. Vendor of the product(s):
GPAC Multimedia Open Source Project

3. Vendor of the product(s) version:
GPAC 2.4

4. Affected product(s)/code base:
MP4Box (media_tools/av_parsers.c)

5. Impact:
Denial of Service

6. Attack type:
Local

7. Affected component(s):
media_tools/av_parsers.c ; gf_hevc_read_sps_bs_internal() ; L:9309

8. Attack vector(s):
Attacker crafts an HEVC-encoded MP4 file containing a malformed SPS NAL unit. When MP4Box imports or splits the file (e.g., using -add or -split), it invokes gf_hevc_read_sps_bs_internal, which reads past allocated memory due to improper bounds checking in SPS parsing.

9. Suggested CVE description:
 A buffer overflow vulnerability exists in the gf_hevc_read_sps_bs_internal function of GPAC's MP4Box (in media_tools/av_parsers.c). Parsing a crafted HEVC Sequence Parameter Set (SPS) causes the parser to read/write beyond allocated buffer limits, leading to memory corruption. A specially constructed MP4 file can trigger this flaw when processed with commands like MP4Box -add or -split, resulting in segmentation faults and potentially arbitrary code execution or denial of service. The root cause is missing proper bounds validation when parsing SPS data.

10. Discoverer(s)/Credits info:
Alexander A. Shvedov (@sigdevel)

11. Reference(s) info:
https://github.com/gpac/gpac/issues/3146
https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/3/3_poc.mp4
https://github.com/makesoftwaresafe/gpac/commit/d091c7e92ef0b6497b808e243501f500135f69c4

12. Additional information:
At this point, the vulnerability is public knowledge and affects all versions of GPAC, including the latest version: 2.4. The developer has proposed a patch to fix this issue, and the fix has been added to the main branch of the repository on GitHub.
1. Reporting to the vendor: https://github.com/gpac/gpac/issues/3146
2. PoC: https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/3/3_poc.mp4
3. Patch: https://github.com/makesoftwaresafe/gpac/commit/d091c7e92ef0b6497b808e243501f500135f69c4
