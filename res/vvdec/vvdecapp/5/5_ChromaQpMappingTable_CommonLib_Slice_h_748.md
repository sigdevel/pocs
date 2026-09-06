[security] CommonLib/Slice.h:748 Use-After-Free in vvdec::ChromaQpMappingTable::~ChromaQpMappingTable

# CommonLib/Slice.h:748 Use-After-Free in vvdec::ChromaQpMappingTable::~ChromaQpMappingTable

#### Description:

When the decoder is closed, `VVDecImpl::uninit` -> `PicListManager::deleteBuffers` (`CommonLib/PicListManager.cpp:107`) destroys every buffered picture. The same `SPS` object (allocated in `DecLibParser::xDecodeSPS`, `DecoderLib/DecLibParser.cpp:1551`) is held through two independent `std::shared_ptr<const SPS>` control blocks wrapping the same raw pointer: one owned by a `Slice` and released in `Picture::clearSliceBuffer` -> `Slice::~Slice` (`CommonLib/Slice.h:2492`), the other owned by a `CodingStructure` and released in `Picture::destroy` -> `CodingStructure::~CodingStructure` (`CommonLib/CodingStructure.h:137`). The first release frees the 7240-byte `SPS`; disposing the second control block then runs `SPS::~SPS` (`CommonLib/Slice.h:1475`) over the freed region, and its member `ChromaQpMappingTable::~ChromaQpMappingTable` (`CommonLib/Slice.h:748`) destroys its `std::vector<int>` fields by reading the freed vector pointers — a heap use-after-free (effectively a double free).


#### To Reproduce

Steps to reproduce the behavior:

```bash
./vvdecapp -b ./5_~ChromaQpMappingTable_CommonLib_Slice_h_748
```

#### Output:

asan-build:

```bash
...
...
...

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "bool vvdec::DecLibParser::xDecodeSliceMain(InputNALUnit &)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:794: duplicate POC in DPB
ERROR CONDITION: m_tmpSeenPocs.count( p->poc ) != 0
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::HLSyntaxReader::parseSliceHeader(Slice *, std::shared_ptr<PicHeader> &, const ParameterSetManager *, const int, bool &)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/HLSyntaxReader.cpp:3480: When sps_subpic_info_present_flag is equal to 1, the value of sh_picture_header_in_slice_header_flag shall be equal to 0
ERROR CONDITION: sps->getSubPicInfoPresentFlag() == 1
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [error]: decoding failed: decoder exception (decoder requires restart) detail: caught decoder exception: 
ERROR: In function "void vvdec::DecLib::sanitizeBrokenPicture(Picture *)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLib.cpp:338: The exception shouldn't have been thrown out already.
ERROR CONDITION: pcParsedPic->exceptionThrownOut

=================================================================
==567060==ERROR: AddressSanitizer: heap-use-after-free on address 0x7d7475e15d20 at pc 0x5611c00c5657 bp 0x7ffebd1b8de0 sp 0x7ffebd1b8dd8
READ of size 8 at 0x7d7475e15d20 thread T0
    #0 0x5611c00c5656 in std::_Vector_base<int, std::allocator<int>>::~_Vector_base() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/stl_vector.h:375:24
    #1 0x5611c00c5656 in std::vector<int, std::allocator<int>>::~vector() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/stl_vector.h:805:7
    #2 0x5611c00c5656 in vvdec::ChromaQpMappingTable::~ChromaQpMappingTable() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Slice.h:748:35
    #3 0x5611c00c5656 in vvdec::SPS::~SPS() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Slice.h:1475:18
    #4 0x5611c0204419 in std::_Sp_counted_ptr<vvdec::SPS*, (__gnu_cxx::_Lock_policy)2>::_M_dispose() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:427:9
    #5 0x5611c00ad9b4 in std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:345:8
    #6 0x5611c00ad9b4 in std::__shared_count<(__gnu_cxx::_Lock_policy)2>::~__shared_count() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:1069:11
    #7 0x5611c00aa854 in std::__shared_ptr<vvdec::SPS const, (__gnu_cxx::_Lock_policy)2>::~__shared_ptr() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:1531:31
    #8 0x5611c00aa854 in vvdec::CodingStructure::~CodingStructure() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/CodingStructure.h:137:35
    #9 0x5611c00b384b in std::default_delete<vvdec::CodingStructure>::operator()(vvdec::CodingStructure*) const /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/unique_ptr.h:92:2
    #10 0x5611c00b384b in std::__uniq_ptr_impl<vvdec::CodingStructure, std::default_delete<vvdec::CodingStructure>>::reset(vvdec::CodingStructure*) /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/unique_ptr.h:204:4
    #11 0x5611c00b384b in std::unique_ptr<vvdec::CodingStructure, std::default_delete<vvdec::CodingStructure>>::reset(vvdec::CodingStructure*) /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/unique_ptr.h:530:7
    #12 0x5611c00b384b in vvdec::Picture::destroy() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Picture.cpp:193:8
    #13 0x5611c00a2e1f in vvdec::PicListManager::deleteBuffers() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/PicListManager.cpp:107:14
    #14 0x5611bff854ae in vvdec::VVDecImpl::uninit() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:195:14
    #15 0x5611bff7e46e in auto vvdec::VVDecImpl::catchExceptions<int (vvdec::VVDecImpl::*)()>(int (vvdec::VVDecImpl::*)()) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.h:249:12
    #16 0x5611bff7e1e7 in vvdec_decoder_close /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdec.cpp:259:22
    #17 0x5611bff53a3e in main /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/App/vvdecapp/vvdecapp.cpp:792:11
    #18 0x7f3476a33f76 in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #19 0x7f3476a34026 in __libc_start_main csu/../csu/libc-start.c:360:3
    #20 0x5611bfe57960 in _start (/run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/build/bin/vvdecapp+0x52e960) (BuildId: 9523b5f8a40e6e4de1a369a93b68eacebf240997)

0x7d7475e15d20 is located 7200 bytes inside of 7240-byte region [0x7d7475e14100,0x7d7475e15d48)
freed by thread T0 here:
    #0 0x5611bff41db6 in operator delete(void*, unsigned long) (/run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/build/bin/vvdecapp+0x618db6) (BuildId: 9523b5f8a40e6e4de1a369a93b68eacebf240997)
    #1 0x5611c00ad9b4 in std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:345:8
    #2 0x5611c00ad9b4 in std::__shared_count<(__gnu_cxx::_Lock_policy)2>::~__shared_count() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:1069:11
    #3 0x5611c00bc663 in std::__shared_ptr<vvdec::SPS const, (__gnu_cxx::_Lock_policy)2>::~__shared_ptr() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:1531:31
    #4 0x5611c00bc663 in vvdec::Slice::~Slice() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Slice.h:2492:7
    #5 0x5611c00b35c8 in vvdec::Picture::clearSliceBuffer() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Picture.cpp:343:5
    #6 0x5611c00b3a12 in vvdec::Picture::destroy() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Picture.cpp:200:3
    #7 0x5611c00a2e1f in vvdec::PicListManager::deleteBuffers() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/PicListManager.cpp:107:14
    #8 0x5611bff854ae in vvdec::VVDecImpl::uninit() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:195:14
    #9 0x5611bff7e46e in auto vvdec::VVDecImpl::catchExceptions<int (vvdec::VVDecImpl::*)()>(int (vvdec::VVDecImpl::*)()) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.h:249:12
    #10 0x5611bff7e1e7 in vvdec_decoder_close /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdec.cpp:259:22
    #11 0x5611bff53a3e in main /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/App/vvdecapp/vvdecapp.cpp:792:11
    #12 0x7f3476a33f76 in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #13 0x7ffebd1baea1  (<unknown module>)

previously allocated by thread T0 here:
    #0 0x5611bff41131 in operator new(unsigned long) (/run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/build/bin/vvdecapp+0x618131) (BuildId: 9523b5f8a40e6e4de1a369a93b68eacebf240997)
    #1 0x5611c01de934 in vvdec::DecLibParser::xDecodeSPS(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:1551:29
    #2 0x5611c01ca2b0 in vvdec::DecLibParser::parse(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:194:5
    #3 0x5611c01b168d in vvdec::DecLib::decode(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLib.cpp:189:29
    #4 0x5611bff8a41e in vvdec::VVDecImpl::decode(vvdecAccessUnit&, vvdecFrame**) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:429:30
    #5 0x5611bff7f458 in auto vvdec::VVDecImpl::catchExceptions<int (vvdec::VVDecImpl::*)(vvdecAccessUnit&, vvdecFrame**), vvdecAccessUnit, vvdecFrame**>(int (vvdec::VVDecImpl::*)(vvdecAccessUnit&, vvdecFrame**), vvdecAccessUnit, vvdecFrame**) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.h:249:12
    #6 0x5611bff7ef67 in vvdec_decode /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdec.cpp:294:13
    #7 0x5611bff4fa78 in main /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/App/vvdecapp/vvdecapp.cpp:741:16
    #8 0x7f3476a33f76 in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #9 0x7ffebd1baea1  (<unknown module>)

SUMMARY: AddressSanitizer: heap-use-after-free /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Slice.h:748:35 in vvdec::ChromaQpMappingTable::~ChromaQpMappingTable()
Shadow bytes around the buggy address:
  0x7d7475e15a80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7d7475e15b00: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7d7475e15b80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7d7475e15c00: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7d7475e15c80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
=>0x7d7475e15d00: fd fd fd fd[fd]fd fd fd fd fa fa fa fa fa fa fa
  0x7d7475e15d80: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7d7475e15e00: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7d7475e15e80: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7d7475e15f00: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7d7475e15f80: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==567060==ABORTING
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

[5_~ChromaQpMappingTable_CommonLib_Slice_h_748](https://github.com/sigdevel/pocs/blob/main/res/vvdec/vvdecapp/5/5_~ChromaQpMappingTable_CommonLib_Slice_h_748)

#### Screenshots

![screen](https://github.com/sigdevel/pocs/blob/main/res/vvdec/vvdecapp/5/5_asan.png?raw=true "screen")

![screen](https://github.com/sigdevel/pocs/blob/main/res/vvdec/vvdecapp/5/5_vanilla.png?raw=true "screen")
