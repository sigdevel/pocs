f5b7cdc63a7f3269040778c5431a8f6c310bc9f3

https://github.com/gpac/gpac/issues/3246

https://github.com/gpac/gpac/commit/63eccc33d4a2b731ebb31581ff5673a2c0b13ad4



1. Vulnerability type: 
NULL Pointer Dereference

2. Vendor of the product(s):
GPAC Multimedia Open Source Project

3. Vendor of the product(s) version:
2.4

4. Affected product(s)/code base:
MP4Box (scenegraph/base_scenegraph.c)

5. Impact:
Denial of Service

6. Attack type:
Local

7. Affected component(s):
scenegraph/base_scenegraph.c ; gf_node_get_tag ; L:1263

8. Attack vector(s):
Processing of specially crafted mp4-files containing invalid BIFS commands (e.g., GlobalQuantizer), corrupted descriptors (tag 4 size 47), and invalid boxes (PEC1808, fre) through the MP4Box -svg command, triggering premature memory deallocation followed by invalid access to freed memory regions.

9. Suggested CVE description:
GPAC's MP4Box contains a use-after-free vulnerability in the gf_node_get_tag function (scenegraph/base_scenegraph.c). When processing files containing malformed BIFS commands, premature node deallocation via gf_node_unregister in bifs/memory_decoder.c leads to invalid memory access. Execution of the -svg command on specially crafted files may cause heap corruption, application crash, or uncontrolled code execution due to invalid pointer dereference.

10. Discoverer(s)/Credits info:
@sigdevel

11. Reference(s) info:
https://github.com/gpac/gpac/issues/3246
https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/20/20_poc.mp4
https://github.com/gpac/gpac/commit/63eccc33d4a2b731ebb31581ff5673a2c0b13ad4

12. Additional information:
At this point, the vulnerability is public knowledge and affects all versions of GPAC, including the latest version: 2.4. The developer has proposed a patch to fix this issue, and the fix has been added to the main branch of the repository on GitHub.
1. Reporting to the vendor: https://github.com/gpac/gpac/issues/3246
2. PoC: hhttps://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/20/20_poc.mp4
3. Patch: https://github.com/gpac/gpac/commit/63eccc33d4a2b731ebb31581ff5673a2c0b13ad4


