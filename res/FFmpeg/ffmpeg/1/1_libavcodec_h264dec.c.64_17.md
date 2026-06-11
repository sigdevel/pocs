### libavcodec/h264dec.c:64:17 SEGV in avpriv_h264_has_num_reorder_frames

#### Description:
When demuxing crafted MPEG‑TS input, ffmpeg’s demuxer may misattribute h.264 data to an Aac AVCodecContext. Probably, this leads to a NULL or invalid ps.sps pointer being passed into avpriv_h264_has_num_reorder_frames, resulting in a SEGV due to an unchecked pointer dereference

#### To Reproduce:
Steps to reproduce the behavior:

```bash
./ffmpeg -i ./1_poc.mp4 -f null -

```

#### Output:

standart-build:
```bash
  built with gcc 14 (Debian 14.2.0-8)
  configuration: --disable-shared --enable-static --disable-doc --enable-gpl --enable-libass --enable-libfreetype --enable-libmp3lame --enable-libopus --enable-libvorbis --enable-libx264 --enable-libx265 --enable-nonfree
  libavutil      60.  3.100 / 60.  3.100
  libavcodec     62.  3.101 / 62.  3.101
  libavformat    62.  0.102 / 62.  0.102
  libavdevice    62.  0.100 / 62.  0.100
  libavfilter    11.  0.100 / 11.  0.100
  libswscale      9.  0.100 /  9.  0.100
  libswresample   6.  0.100 /  6.  0.100
Trailing option(s) found in the command: may be ignored.
[mpegts @ 0x56494ad1c880] Packet corrupt (stream = 0, dts = 126000).                                                                                                                                                                        
[aac @ 0x56494ad5e0c0] This AVCodecContext was allocated for aac, but h264 passed to avcodec_open2()                                                                                                                                        
[aac @ 0x56494ad5e0c0] reference count 2 overflow                                                                                                                                                                                           
zsh: segmentation fault (core dumped)   -i  -f null ```

asan-build:
<details>
  <summary>show full -click to expand</summary>
  
```bash
  built with Debian clang version 19.1.7 (1+b1)
  configuration: --disable-shared --enable-static --disable-doc --enable-gpl --enable-libass --enable-libfreetype --enable-libmp3lame --enable-libopus --enable-libvorbis --enable-libx264 --enable-libx265 --enable-nonfree --toolchain=clang-asan --enable-debug=3 --disable-optimizations --disable-stripping
  libavutil      60.  3.100 / 60.  3.100
  libavcodec     62.  3.101 / 62.  3.101
  libavformat    62.  0.102 / 62.  0.102
  libavdevice    62.  0.100 / 62.  0.100
  libavfilter    11.  0.100 / 11.  0.100
  libswscale      9.  0.100 /  9.  0.100
  libswresample   6.  0.100 /  6.  0.100
Trailing option(s) found in the command: may be ignored.
[mpegts @ 0x517000000080] Packet corrupt (stream = 0, dts = 126000).                                                                                                                                                                        
[aac @ 0x519000002380] This AVCodecContext was allocated for aac, but h264 passed to avcodec_open2()                                                                                                                                        
[aac @ 0x519000002380] reference count 2 overflow                                                                                                                                                                                           
AddressSanitizer:DEADLYSIGNAL                                                                                                                                                                                                               
=================================================================
==2638652==ERROR: AddressSanitizer: SEGV on unknown address 0x52d000103a40 (pc 0x5637cc943562 bp 0x7ffedf4d66e0 sp 0x7ffedf4d66a0 T0)
==2638652==The signal is caused by a READ memory access.                                                                                                                                                                                    
    #0 0x5637cc943562 in avpriv_h264_has_num_reorder_frames /media/user/6d3eeb8a-a93b-4220-bb13-a4e488ce0ce2/gpac/runtime/ffmpeg_asan/libavcodec/h264dec.c:64:17
    #1 0x5637cbb65978 in has_decode_delay_been_guessed /media/user/6d3eeb8a-a93b-4220-bb13-a4e488ce0ce2/gpac/runtime/ffmpeg_asan/libavformat/demux.c:757:9
    #2 0x5637cbb6e7e4 in compute_pkt_fields /media/user/6d3eeb8a-a93b-4220-bb13-a4e488ce0ce2/gpac/runtime/ffmpeg_asan/libavformat/demux.c:1137:13
    #3 0x5637cbb6b52c in parse_packet /media/user/6d3eeb8a-a93b-4220-bb13-a4e488ce0ce2/gpac/runtime/ffmpeg_asan/libavformat/demux.c:1265:9
    #4 0x5637cbb56b45 in read_frame_internal /media/user/6d3eeb8a-a93b-4220-bb13-a4e488ce0ce2/gpac/runtime/ffmpeg_asan/libavformat/demux.c:1449:24
    #5 0x5637cbb5cfe6 in avformat_find_stream_info /media/user/6d3eeb8a-a93b-4220-bb13-a4e488ce0ce2/gpac/runtime/ffmpeg_asan/libavformat/demux.c:2692:15
    #6 0x5637cacc47d0 in ifile_open /media/user/6d3eeb8a-a93b-4220-bb13-a4e488ce0ce2/gpac/runtime/ffmpeg_asan/fftools/ffmpeg_demux.c:1814:15
    #7 0x5637cad28f84 in open_files /media/user/6d3eeb8a-a93b-4220-bb13-a4e488ce0ce2/gpac/runtime/ffmpeg_asan/fftools/ffmpeg_opt.c:1366:15
    #8 0x5637cad289c8 in ffmpeg_parse_options /media/user/6d3eeb8a-a93b-4220-bb13-a4e488ce0ce2/gpac/runtime/ffmpeg_asan/fftools/ffmpeg_opt.c:1415:11
    #9 0x5637cad6c059 in main /media/user/6d3eeb8a-a93b-4220-bb13-a4e488ce0ce2/gpac/runtime/ffmpeg_asan/fftools/ffmpeg.c:991:11
    #10 0x7f3ccb833ca7 in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #11 0x7f3ccb833d64 in __libc_start_main csu/../csu/libc-start.c:360:3
    #12 0x5637cabcd710 in _start (/media/user/6d3eeb8a-a93b-4220-bb13-a4e488ce0ce2/gpac/runtime/ffmpeg_asan/ffmpeg+0x520710) (BuildId: a3aa18ba38f6965f4a464a3b067a40b740dc1cf2)

==2638652==Register values:
rax = 0x000052d000103a40  rbx = 0x00007ffedf4d67e0  rcx = 0x0000000000000001  rdx = 0x0000000000000040  
rdi = 0x0000519000002380  rsi = 0x00000000000000f5  rbp = 0x00007ffedf4d66e0  rsp = 0x00007ffedf4d66a0  
 r8 = 0x00007fffffffff01   r9 = 0x0000000000000701  r10 = 0x00000fffdbe9ae3c  r11 = 0x000010005be92e38  
r12 = 0x0000000000000000  r13 = 0x00007ffedf4da838  r14 = 0x00007f3ccda67000  r15 = 0x00005637cf83b1d0  
AddressSanitizer can not provide additional info.
SUMMARY: AddressSanitizer: SEGV /media/user/6d3eeb8a-a93b-4220-bb13-a4e488ce0ce2/gpac/runtime/ffmpeg_asan/libavcodec/h264dec.c:64:17 in avpriv_h264_has_num_reorder_frames
==2638652==ABORTING

```

</details>


#### Expected behavior:

Probably, ffmpeg should not crash or return SEGV when encountering corrupted streams, instead:
- detect invalid or mismatched codec contexts and return an error ;
- verify non-null ps.sps before accessing it ;
- skip corumpted frames and continue demuxing or fail with determined error

Impact:

Specially crafted media file can cause ffmpeg to crash via a SEGV, such behavior may be exploited for denial-of-service or potential remote code execution if compiled without memory sanitizers and with exploitable memory corruption.

#### Environment

    OS: tested at Kali 6.11.2-1kali1 (2024-10-15) x86_64 GNU/Linux ;
    Compiler version: Debian clang version 19.1.7 (1+b1) / gcc 14 (Debian 14.2.0-8);
    CPU type: x86_64 ;
    FFmpeg - ffmpeg commit hash be46370941405fb04402d96373a53e2a1846f3ac
    Build opts debug: --disable-shared --enable-static --disable-doc --enable-gpl --enable-libass --enable-libfreetype --enable-libmp3lame --enable-libopus --enable-libvorbis --enable-libx264 --enable-libx265 --enable-nonfree --toolchain=clang-asan --enable-debug=3 --disable-optimizations --disable-stripping ;
    Build opts standart: --disable-shared --enable-static --disable-doc --enable-gpl --enable-libass --enable-libfreetype --enable-libmp3lame --enable-libopus --enable-libvorbis --enable-libx264 --enable-libx265 --enable-nonfree

#### Additional context

link to the sample (github-url):

[1_poc.mp4](https://github.com/sigdevel/pocs/blob/main/res/FFmpeg/ffmpeg/1_poc.mp4)

#### Screenshots
![screen](https://github.com/sigdevel/pocs/blob/main/res/FFmpeg/ffmpeg/ffpmeg_1+2025-06-08_17-07.png?raw=true "screen")

![screen](https://github.com/sigdevel/pocs/blob/main/res/FFmpeg/ffmpeg/ffpmeg_1_clean+2025-06-08_17-07.png?raw=true "screen")
