027ce139dda498ee95df36db9f9f6f3cadce8ec9

https://github.com/gpac/gpac/issues/3261

https://github.com/gpac/gpac/commit/321624f28d19a413449fd1718d1eb59037f8f7fc


1. Vulnerability type: Heap-based Buffer Overflow

2. Vendor of the product(s):
GPAC Multimedia Open Source Project

3. Vendor of the product(s) version:
2.4

4. Affected product(s)/code base:
MP4Box (isomedia/stbl_write.c)

5. Impact:
Code Execution, Denial of Service

6. Attack type:
Local

7. Affected component(s):
isomedia/stbl_write.c ; stbl_AddSize ; L:492 ; (stbl->sampleSize->sizes[stbl->sampleSize->sampleCount] = size;)

8. Attack vector(s):
Processing mp4-files containing manipulated sample metadata (corrupted sample counts, invalid aspect ratios, and oversized box declarations) through MP4Box's -add command. The vulnerability triggers during sample size table population when the application writes beyond allocated heap buffer boundaries due to insufficient index validation

9. Suggested CVE description:
A heap-based buffer overflow vulnerability exists in the stbl_AddSize function of GPAC's MP4Box (isomedia/stbl_write.c). When handling specially crafted MP4 files with abnormal sample metadata, the application fails to validate sample count boundaries during buffer write operations. Malicious files processed via commands like -add can trigger out-of-bounds memory writes, potentially leading to arbitrary code execution or denial of service. The vulnerability originates from missing bounds checks in sample size array updates. Successful exploitation requires file processing by the application.

10. Discoverer(s)/Credits info:
@sigdevel

11. Reference(s) info:
https://github.com/gpac/gpac/issues/3261
https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/25/25_poc.mp4
https://github.com/gpac/gpac/commit/321624f28d19a413449fd1718d1eb59037f8f7fc

12. Additional information:
At this point, the vulnerability is public knowledge and affects all versions of GPAC, including the latest version: 2.4. The developer has proposed a patch to fix this issue, and the fix has been added to the main branch of the repository on GitHub.
1. Reporting to the vendor: https://github.com/gpac/gpac/issues/3261
2. PoC: https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/25/25_poc.mp4
3. Patch: https://github.com/gpac/gpac/commit/321624f28d19a413449fd1718d1eb59037f8f7fc
