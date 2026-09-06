[security] CommonLib/Slice.h:1485 Use-After-Free in vvdec::SPS::getChromaFormatIdc

# CommonLib/Slice.h:1485 Use-After-Free in vvdec::SPS::getChromaFormatIdc

#### Description:

While activating parameter sets for a new slice, `DecLibParser::xActivateParameterSets` (`DecoderLib/DecLibParser.cpp:1056`) recycles a picture buffer via `PicListManager::getNewPicBuffer` -> `Picture::resetForUse` -> `clearSliceBuffer` -> `Slice::~Slice` (`CommonLib/Slice.h:2492`), which releases the last `std::shared_ptr<const SPS>` to the active `SPS` and frees the 7240-byte object (allocated in `DecLibParser::xDecodeSPS`, `DecoderLib/DecLibParser.cpp:1551`). Immediately afterwards, at `DecLibParser.cpp:1062`, the same function calls `Picture::finalInit` (`CommonLib/Picture.cpp:248`) with the now-dangling `SPS` pointer, and `SPS::getChromaFormatIdc` (`CommonLib/Slice.h:1485`) reads `m_chromaFormatIdc` from the freed region — a heap use-after-free. A crafted VVC bitstream that forces a picture-buffer recycle while the active SPS is still referenced triggers the bug.


#### To Reproduce

Steps to reproduce the behavior:

```bash
./vvdecapp -b ./6_getChromaFormatIdc_CommonLib_Slice_h_1485
```

#### Output:

asan-build:

```bash
vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "auto vvdec::HLSyntaxReader::parsePictureHeader(PicHeader *, const ParameterSetManager *, bool)::(anonymous class)::operator()() const" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/HLSyntaxReader.cpp:3033: ph_log2_diff_min_qt_min_cb_intra_slice_chroma out of bounds (read:6).
ERROR CONDITION: (ph_log2_diff_min_qt_min_cb_intra_slice_chroma) < (0) || (ph_log2_diff_min_qt_min_cb_intra_slice_chroma) > (std::min( 6u, CtbLog2SizeY ) - MinCbLog2SizeY)
You can try to pass in more data to start decoding from the first RAP.


WARNING: In function "static int vvdec::VVDecImpl::xReadNalUnitHeader(InputNALUnit &)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:1490: forbidden_zero_bit shall be equal to 0.
WARNING CONDITION: nalu.m_forbiddenZeroBit != 0

WARNING: In function "static int vvdec::VVDecImpl::xReadNalUnitHeader(InputNALUnit &)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:1493: nuh_reserved_zero_bit shall be equal to 0.
WARNING CONDITION: nalu.m_forbiddenZeroBit != 0
vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "auto vvdec::HLSyntaxReader::parsePictureHeader(PicHeader *, const ParameterSetManager *, bool)::(anonymous class)::operator()() const" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/HLSyntaxReader.cpp:3052: ph_log2_diff_max_tt_min_qt_intra_slice_chroma out of bounds (read:5).
ERROR CONDITION: (ph_log2_diff_max_tt_min_qt_intra_slice_chroma) < (0) || (ph_log2_diff_max_tt_min_qt_intra_slice_chroma) > (std::min( 6u, CtbLog2SizeY ) - MinQtLog2SizeIntraC)
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::DecSlice::parseSlice(Slice *, InputBitstream *, int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecSlice.cpp:165: Expecting a terminating bit
ERROR CONDITION: !binVal
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::DecSlice::parseSlice(Slice *, InputBitstream *, int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecSlice.cpp:165: Expecting a terminating bit
ERROR CONDITION: !binVal
You can try to pass in more data to start decoding from the first RAP.

=================================================================
==610247==ERROR: AddressSanitizer: heap-use-after-free on address 0x7d62383e212a at pc 0x5614b068c8d3 bp 0x7ffd4584ac30 sp 0x7ffd4584ac28                                                                                                             
READ of size 1 at 0x7d62383e212a thread T0                                                                                 
    #0 0x5614b068c8d2 in vvdec::SPS::getChromaFormatIdc() const /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Slice.h:1485:116
    #1 0x5614b068c8d2 in vvdec::Picture::finalInit(vvdec::thread_safe_chunk_cache<vvdec::CodingUnit>*, vvdec::thread_safe_chunk_cache<vvdec::TransformUnit>*, vvdec::SPS const*, vvdec::PPS const*, std::shared_ptr<vvdec::PicHeader> const&, vvdec::APS const* const*, vvdec::APS const*, vvdec::APS const*, bool) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Picture.cpp:248:45
    #2 0x5614b07b9f97 in vvdec::DecLibParser::xActivateParameterSets(int) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:1062:19
    #3 0x5614b07a52d3 in vvdec::DecLibParser::xDecodeSliceHead(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:640:3
    #4 0x5614b079d7e4 in vvdec::DecLibParser::parse(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:155:12
    #5 0x5614b078468d in vvdec::DecLib::decode(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLib.cpp:189:29
    #6 0x5614b055d41e in vvdec::VVDecImpl::decode(vvdecAccessUnit&, vvdecFrame**) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:429:30
    #7 0x5614b0552458 in auto vvdec::VVDecImpl::catchExceptions<int (vvdec::VVDecImpl::*)(vvdecAccessUnit&, vvdecFrame**), vvdecAccessUnit, vvdecFrame**>(int (vvdec::VVDecImpl::*)(vvdecAccessUnit&, vvdecFrame**), vvdecAccessUnit, vvdecFrame**) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.h:249:12
    #8 0x5614b0551f67 in vvdec_decode /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdec.cpp:294:13
    #9 0x5614b0522a78 in main /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/App/vvdecapp/vvdecapp.cpp:741:16
    #10 0x7f2239033f76 in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #11 0x7f2239034026 in __libc_start_main csu/../csu/libc-start.c:360:3
    #12 0x5614b042a960 in _start (/run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/build/bin/vvdecapp+0x52e960) (BuildId: 9523b5f8a40e6e4de1a369a93b68eacebf240997)

0x7d62383e212a is located 42 bytes inside of 7240-byte region [0x7d62383e2100,0x7d62383e3d48)
freed by thread T0 here:                                                                                                   
    #0 0x5614b0514db6 in operator delete(void*, unsigned long) (/run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/build/bin/vvdecapp+0x618db6) (BuildId: 9523b5f8a40e6e4de1a369a93b68eacebf240997)
    #1 0x5614b06809b4 in std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:345:8
    #2 0x5614b06809b4 in std::__shared_count<(__gnu_cxx::_Lock_policy)2>::~__shared_count() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:1069:11
    #3 0x5614b068f663 in std::__shared_ptr<vvdec::SPS const, (__gnu_cxx::_Lock_policy)2>::~__shared_ptr() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:1531:31
    #4 0x5614b068f663 in vvdec::Slice::~Slice() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Slice.h:2492:7
    #5 0x5614b06865c8 in vvdec::Picture::clearSliceBuffer() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Picture.cpp:343:5
    #6 0x5614b0685c3f in vvdec::Picture::resetForUse(int) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Picture.cpp:174:3
    #7 0x5614b067838c in vvdec::PicListManager::getNewPicBuffer(vvdec::SPS const&, vvdec::PPS const&, unsigned int, int, vvdec::VPS const*) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/PicListManager.cpp:229:10
    #8 0x5614b07b9cc8 in vvdec::DecLibParser::xActivateParameterSets(int) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:1056:37
    #9 0x5614b07a52d3 in vvdec::DecLibParser::xDecodeSliceHead(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:640:3
    #10 0x5614b079d7e4 in vvdec::DecLibParser::parse(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:155:12
    #11 0x5614b078468d in vvdec::DecLib::decode(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLib.cpp:189:29
    #12 0x5614b055d41e in vvdec::VVDecImpl::decode(vvdecAccessUnit&, vvdecFrame**) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:429:30
    #13 0x5614b0552458 in auto vvdec::VVDecImpl::catchExceptions<int (vvdec::VVDecImpl::*)(vvdecAccessUnit&, vvdecFrame**), vvdecAccessUnit, vvdecFrame**>(int (vvdec::VVDecImpl::*)(vvdecAccessUnit&, vvdecFrame**), vvdecAccessUnit, vvdecFrame**) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.h:249:12
    #14 0x5614b0551f67 in vvdec_decode /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdec.cpp:294:13
    #15 0x5614b0522a78 in main /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/App/vvdecapp/vvdecapp.cpp:741:16
    #16 0x7f2239033f76 in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #17 0x7ffd4584be9f  (<unknown module>)

previously allocated by thread T0 here:
    #0 0x5614b0514131 in operator new(unsigned long) (/run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/build/bin/vvdecapp+0x618131) (BuildId: 9523b5f8a40e6e4de1a369a93b68eacebf240997)
    #1 0x5614b07b1934 in vvdec::DecLibParser::xDecodeSPS(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:1551:29
    #2 0x5614b079d2b0 in vvdec::DecLibParser::parse(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:194:5
    #3 0x5614b078468d in vvdec::DecLib::decode(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLib.cpp:189:29
    #4 0x5614b055d41e in vvdec::VVDecImpl::decode(vvdecAccessUnit&, vvdecFrame**) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:429:30
    #5 0x5614b0552458 in auto vvdec::VVDecImpl::catchExceptions<int (vvdec::VVDecImpl::*)(vvdecAccessUnit&, vvdecFrame**), vvdecAccessUnit, vvdecFrame**>(int (vvdec::VVDecImpl::*)(vvdecAccessUnit&, vvdecFrame**), vvdecAccessUnit, vvdecFrame**) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.h:249:12
    #6 0x5614b0551f67 in vvdec_decode /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdec.cpp:294:13
    #7 0x5614b0522a78 in main /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/App/vvdecapp/vvdecapp.cpp:741:16
    #8 0x7f2239033f76 in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #9 0x7ffd4584be9f  (<unknown module>)

SUMMARY: AddressSanitizer: heap-use-after-free /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Slice.h:1485:116 in vvdec::SPS::getChromaFormatIdc() const
Shadow bytes around the buggy address:
  0x7d62383e1e80: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7d62383e1f00: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7d62383e1f80: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7d62383e2000: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7d62383e2080: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
=>0x7d62383e2100: fd fd fd fd fd[fd]fd fd fd fd fd fd fd fd fd fd
  0x7d62383e2180: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7d62383e2200: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7d62383e2280: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7d62383e2300: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7d62383e2380: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==610247==ABORTING
```

#### Environment

```text
OS: tested at 7.1.5+kali-amd64 x86_64 GNU/Linux ;
Compiler version: Debian clang version 21.1.8 ;
CPU type: x86_64 ;
VVdeC - vvdecapp commit hash e493ce51f13a2dea72cd58354652ed4e0f509a0e ;
VVdeC - vvdecapp version 3.3.0-dev ;
Build flags: clang RelWithDebInfo, -DBUILD_SHARED_LIBS=OFF -DVVDEC_TOPLEVEL_OUTPUT_DIRS=OFF -DVVDEC_ENABLE_LINK_TIME_OPT=OFF -DVVDEC_ENABLE_WERROR=OFF -DVVDEC_ENABLE_X86_SIMD=OFF -DVVDEC_FUZZING_BUILD=OFF ;
Asan build flags: clang RelWithDebInfo, -DBUILD_SHARED_LIBS=OFF -DVVDEC_TOPLEVEL_OUTPUT_DIRS=OFF -DVVDEC_ENABLE_LINK_TIME_OPT=OFF -DVVDEC_USE_ADDRESS_SANITIZER=ON -DVVDEC_FUZZING_BUILD=ON ;
```

#### Additional context

link to the sample (github-url):

[6_getChromaFormatIdc_CommonLib_Slice_h_1485](https://github.com/sigdevel/pocs/blob/main/res/vvdec/vvdecapp/6/6_getChromaFormatIdc_CommonLib_Slice_h_1485)

#### Screenshots

![screen](https://github.com/sigdevel/pocs/blob/main/res/vvdec/vvdecapp/6/6_asan.png?raw=true "screen")

![screen](https://github.com/sigdevel/pocs/blob/main/res/vvdec/vvdecapp/6/6_vanilla.png?raw=true "screen")
