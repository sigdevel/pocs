GPAC version 2.5-DEV-rev1174-g3017379f1-master

https://github.com/gpac/gpac/issues/3129

https://github.com/gpac/gpac/commit/bc2fd5bb5c31ae148624767c4a7a17f02c42951b


1. Vulnerability type: Buffer Overflow

2. Vendor of the product(s):
GPAC Multimedia Open Source Project

3. Vendor of the product(s) version:
GPAC 2.4

4. Affected product(s)/code base:
MP4Box (filters/in_file.c)

5. Impact:
Code Execution

6. Attack type:
Local

7. Affected component(s):
filters/in_file.c ; filein_process() ; L:700 (char szStatus[1024])

8. Attack vector(s):
The attacker generates an MP4 file with a long or specially chosen path name (ctx->src) containing a very long string or non-standard characters, causing a status string overflow and writing more than 1024 bytes to szStatus[1024].

9. Suggested CVE description:
 The filein_process MP4Box function (part of GPAC) uses an unsafe sprintf call to generate a status string. When processing files with long paths or anomalous characters, the status string may exceed the size of the szStatus[1024] buffer, causing a stack overflow. This can result in application denial of service or potential exploitation with arbitrary code execution. The vulnerability occurs when MP4Box is run with the status parameter (DASH), and injection of a malicious MP4 file could result in a crash or compromise.

10. Discoverer(s)/Credits info:
Alexander A. Shvedov (@sigdevel)

11. Reference(s) info:
https://github.com/gpac/gpac/issues/3129
https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/1/1_poc.mp4
https://github.com/gpac/gpac/commit/bc2fd5bb5c31ae148624767c4a7a17f02c42951b

12. Additional information:
At this point, the vulnerability is public knowledge and affects all versions of GPAC, including the latest version: 2.4. The developer has proposed a patch to fix this issue, and the fix has been added to the main branch of the repository on GitHub.
1. Reporting to the vendor: https://github.com/gpac/gpac/issues/3129
2. PoC: https://github.com/sigdevel/pocs/blob/main/res/gpac/MP4Box/1/1_poc.mp4
3. Patch: https://github.com/gpac/gpac/commit/bc2fd5bb5c31ae148624767c4a7a17f02c42951b
