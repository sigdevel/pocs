[security] CommonLib/Slice.h:1957 Use-After-Free in vvdec::PPS::~PPS

# CommonLib/Slice.h:1957 Use-After-Free in vvdec::PPS::~PPS

#### Description:

When the decoder is closed, `VVDecImpl::uninit` -> `PicListManager::deleteBuffers` (`CommonLib/PicListManager.cpp:107`) destroys every buffered picture. The same `PPS` object (allocated in `DecLibParser::xDecodePPS`, `DecoderLib/DecLibParser.cpp:1561`) is held through two independent `std::shared_ptr<const PPS>` control blocks, each wrapping the same raw pointer: one owned by a `Slice` and released in `Picture::clearSliceBuffer` -> `Slice::~Slice` (`CommonLib/Slice.h:2492`), the other owned by a `CodingStructure` and released in `Picture::destroy` -> `CodingStructure::~CodingStructure` (`CommonLib/CodingStructure.h:137`). The first release frees the 1840-byte `PPS`; disposing the second control block then runs `PPS::~PPS` (`CommonLib/Slice.h:1957`) over the already-freed region and reads the freed `std::unique_ptr<PreCalcValues>` member — a heap use-after-free (effectively a double free). A crafted VVC bitstream that leaves one parameter set owned through more than one control block triggers the defect at decoder shutdown.


#### To Reproduce

Steps to reproduce the behavior:

```bash
./vvdecapp -b ./4_~PPS_CommonLib_Slice_h_1957
```

#### Output:

asan-build:

```bash
vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "const T *vvdec::ParameterSetMap<vvdec::PPS, 64>::getPS(int) const [T = vvdec::PPS, MAX_ID = 64]" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/../CommonLib/ParameterSetManager.h:139: Missing Parameter Set (id:1)
ERROR CONDITION: !ps
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::InputBitstream::load_next_bits(int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/BitStream.h:193: Exceeded FIFO size
ERROR CONDITION: m_fifo_idx + required_bytes > m_fifo.size()
You can try to pass in more data to start decoding from the first RAP.


WARNING: In function "static int vvdec::VVDecImpl::xReadNalUnitHeader(InputNALUnit &)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:1490: forbidden_zero_bit shall be equal to 0.
WARNING CONDITION: nalu.m_forbiddenZeroBit != 0

WARNING: In function "static int vvdec::VVDecImpl::xReadNalUnitHeader(InputNALUnit &)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:1493: nuh_reserved_zero_bit shall be equal to 0.
WARNING CONDITION: nalu.m_forbiddenZeroBit != 0
vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::InputBitstream::load_next_bits(int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/BitStream.h:193: Exceeded FIFO size
ERROR CONDITION: m_fifo_idx + required_bytes > m_fifo.size()
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::InputBitstream::load_next_bits(int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/BitStream.h:193: Exceeded FIFO size
ERROR CONDITION: m_fifo_idx + required_bytes > m_fifo.size()
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::InputBitstream::load_next_bits(int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/BitStream.h:193: Exceeded FIFO size
ERROR CONDITION: m_fifo_idx + required_bytes > m_fifo.size()
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "auto vvdec::HLSyntaxReader::parseSliceHeader(Slice *, std::shared_ptr<PicHeader> &, const ParameterSetManager *, const int, bool &)::(anonymous class)::operator()() const" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/HLSyntaxReader.cpp:3832: sh_collocated_ref_idx out of bounds (read:4).
ERROR CONDITION: (sh_collocated_ref_idx) < (0) || (sh_collocated_ref_idx) > (pcSlice->getNumRefIdx( pcSlice->getColFromL0Flag() ? REF_PIC_LIST_0 : REF_PIC_LIST_1 ) - 1u)
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::InputBitstream::load_next_bits(int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/BitStream.h:193: Exceeded FIFO size
ERROR CONDITION: m_fifo_idx + required_bytes > m_fifo.size()
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::InputBitstream::load_next_bits(int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/BitStream.h:193: Exceeded FIFO size
ERROR CONDITION: m_fifo_idx + required_bytes > m_fifo.size()
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::InputBitstream::load_next_bits(int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/BitStream.h:193: Exceeded FIFO size
ERROR CONDITION: m_fifo_idx + required_bytes > m_fifo.size()
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::InputBitstream::load_next_bits(int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/BitStream.h:193: Exceeded FIFO size
ERROR CONDITION: m_fifo_idx + required_bytes > m_fifo.size()
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::InputBitstream::load_next_bits(int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/BitStream.h:193: Exceeded FIFO size
ERROR CONDITION: m_fifo_idx + required_bytes > m_fifo.size()
You can try to pass in more data to start decoding from the first RAP.


WARNING: In function "static int vvdec::VVDecImpl::xReadNalUnitHeader(InputNALUnit &)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:1490: forbidden_zero_bit shall be equal to 0.
WARNING CONDITION: nalu.m_forbiddenZeroBit != 0

WARNING: In function "static int vvdec::VVDecImpl::xReadNalUnitHeader(InputNALUnit &)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:1493: nuh_reserved_zero_bit shall be equal to 0.
WARNING CONDITION: nalu.m_forbiddenZeroBit != 0
vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::DecSlice::parseSlice(Slice *, InputBitstream *, int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecSlice.cpp:165: Expecting a terminating bit
ERROR CONDITION: !binVal
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "void vvdec::InputBitstream::load_next_bits(int)" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/BitStream.h:193: Exceeded FIFO size
ERROR CONDITION: m_fifo_idx + required_bytes > m_fifo.size()
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [warning]: (possibly recoverable) exception (decoder input data error) detail: Exception while tuning in: 
ERROR: In function "const T *vvdec::ParameterSetMap<vvdec::SPS, 16>::getPS(int) const [T = vvdec::SPS, MAX_ID = 16]" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/../CommonLib/ParameterSetManager.h:139: Missing Parameter Set (id:1)
ERROR CONDITION: !ps
You can try to pass in more data to start decoding from the first RAP.

vvdecapp [error]: decoding failed: (possibly recoverable) exception (unspecified malfunction) detail: Exception while tuning in: 
ERROR: In function "const T *vvdec::ParameterSetMap<vvdec::SPS, 16>::getPS(int) const [T = vvdec::SPS, MAX_ID = 16]" in /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/../CommonLib/ParameterSetManager.h:139: Missing Parameter Set (id:1)
ERROR CONDITION: !ps
You can try to pass in more data to start decoding from the first RAP.

=================================================================
==562338==ERROR: AddressSanitizer: heap-use-after-free on address 0x7d1b12be47a8 at pc 0x56177e3ac6f4 bp 0x7ffcb368dd40 sp 0x7ffcb368dd38                                                                                                             
READ of size 8 at 0x7d1b12be47a8 thread T0                                                                                 
    #0 0x56177e3ac6f3 in std::unique_ptr<vvdec::PreCalcValues, std::default_delete<vvdec::PreCalcValues>>::~unique_ptr() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/unique_ptr.h:407:6
    #1 0x56177e3ac6f3 in vvdec::PPS::~PPS() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Slice.h:1957:18
    #2 0x56177e4ecbc9 in std::_Sp_counted_ptr<vvdec::PPS*, (__gnu_cxx::_Lock_policy)2>::_M_dispose() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:427:9
    #3 0x56177e3939b4 in std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:345:8
    #4 0x56177e3939b4 in std::__shared_count<(__gnu_cxx::_Lock_policy)2>::~__shared_count() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:1069:11
    #5 0x56177e390848 in std::__shared_ptr<vvdec::PPS const, (__gnu_cxx::_Lock_policy)2>::~__shared_ptr() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:1531:31
    #6 0x56177e390848 in vvdec::CodingStructure::~CodingStructure() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/CodingStructure.h:137:35
    #7 0x56177e39984b in std::default_delete<vvdec::CodingStructure>::operator()(vvdec::CodingStructure*) const /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/unique_ptr.h:92:2
    #8 0x56177e39984b in std::__uniq_ptr_impl<vvdec::CodingStructure, std::default_delete<vvdec::CodingStructure>>::reset(vvdec::CodingStructure*) /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/unique_ptr.h:204:4
    #9 0x56177e39984b in std::unique_ptr<vvdec::CodingStructure, std::default_delete<vvdec::CodingStructure>>::reset(vvdec::CodingStructure*) /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/unique_ptr.h:530:7
    #10 0x56177e39984b in vvdec::Picture::destroy() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Picture.cpp:193:8
    #11 0x56177e388e1f in vvdec::PicListManager::deleteBuffers() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/PicListManager.cpp:107:14
    #12 0x56177e26b4ae in vvdec::VVDecImpl::uninit() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:195:14
    #13 0x56177e26446e in auto vvdec::VVDecImpl::catchExceptions<int (vvdec::VVDecImpl::*)()>(int (vvdec::VVDecImpl::*)()) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.h:249:12
    #14 0x56177e2641e7 in vvdec_decoder_close /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdec.cpp:259:22
    #15 0x56177e239a3e in main /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/App/vvdecapp/vvdecapp.cpp:792:11
    #16 0x7f5b138fdf76 in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #17 0x7f5b138fe026 in __libc_start_main csu/../csu/libc-start.c:360:3
    #18 0x56177e13d960 in _start (/run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/build/bin/vvdecapp+0x52e960) (BuildId: 9523b5f8a40e6e4de1a369a93b68eacebf240997)

0x7d1b12be47a8 is located 1832 bytes inside of 1840-byte region [0x7d1b12be4080,0x7d1b12be47b0)
freed by thread T0 here:                                                                                                   
    #0 0x56177e227db6 in operator delete(void*, unsigned long) (/run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/build/bin/vvdecapp+0x618db6) (BuildId: 9523b5f8a40e6e4de1a369a93b68eacebf240997)
    #1 0x56177e3939b4 in std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:345:8
    #2 0x56177e3939b4 in std::__shared_count<(__gnu_cxx::_Lock_policy)2>::~__shared_count() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:1069:11
    #3 0x56177e3a2657 in std::__shared_ptr<vvdec::PPS const, (__gnu_cxx::_Lock_policy)2>::~__shared_ptr() /usr/lib/gcc/x86_64-linux-gnu/15/../../../../include/c++/15/bits/shared_ptr_base.h:1531:31
    #4 0x56177e3a2657 in vvdec::Slice::~Slice() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Slice.h:2492:7
    #5 0x56177e3995c8 in vvdec::Picture::clearSliceBuffer() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Picture.cpp:343:5
    #6 0x56177e399a12 in vvdec::Picture::destroy() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Picture.cpp:200:3
    #7 0x56177e388e1f in vvdec::PicListManager::deleteBuffers() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/PicListManager.cpp:107:14
    #8 0x56177e26b4ae in vvdec::VVDecImpl::uninit() /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:195:14
    #9 0x56177e26446e in auto vvdec::VVDecImpl::catchExceptions<int (vvdec::VVDecImpl::*)()>(int (vvdec::VVDecImpl::*)()) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.h:249:12
    #10 0x56177e2641e7 in vvdec_decoder_close /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdec.cpp:259:22
    #11 0x56177e239a3e in main /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/App/vvdecapp/vvdecapp.cpp:792:11
    #12 0x7f5b138fdf76 in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #13 0x7ffcb368ee9f  (<unknown module>)

previously allocated by thread T0 here:
    #0 0x56177e227131 in operator new(unsigned long) (/run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/build/bin/vvdecapp+0x618131) (BuildId: 9523b5f8a40e6e4de1a369a93b68eacebf240997)
    #1 0x56177e4c4c04 in vvdec::DecLibParser::xDecodePPS(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:1561:29
    #2 0x56177e4b014a in vvdec::DecLibParser::parse(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLibParser.cpp:198:5
    #3 0x56177e49768d in vvdec::DecLib::decode(vvdec::InputNALUnit&) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/DecoderLib/DecLib.cpp:189:29
    #4 0x56177e27041e in vvdec::VVDecImpl::decode(vvdecAccessUnit&, vvdecFrame**) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.cpp:429:30
    #5 0x56177e265458 in auto vvdec::VVDecImpl::catchExceptions<int (vvdec::VVDecImpl::*)(vvdecAccessUnit&, vvdecFrame**), vvdecAccessUnit, vvdecFrame**>(int (vvdec::VVDecImpl::*)(vvdecAccessUnit&, vvdecFrame**), vvdecAccessUnit, vvdecFrame**) /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdecimpl.h:249:12
    #6 0x56177e264f67 in vvdec_decode /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/vvdec/vvdec.cpp:294:13
    #7 0x56177e235a78 in main /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/App/vvdecapp/vvdecapp.cpp:741:16
    #8 0x7f5b138fdf76 in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #9 0x7ffcb368ee9f  (<unknown module>)

SUMMARY: AddressSanitizer: heap-use-after-free /run/media/user/8ed8205b-4114-4c2a-b2d0-e2ad6640262d/vvdec/vvdec_asan/source/Lib/CommonLib/Slice.h:1957:18 in vvdec::PPS::~PPS()
Shadow bytes around the buggy address:
  0x7d1b12be4500: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7d1b12be4580: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7d1b12be4600: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7d1b12be4680: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7d1b12be4700: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
=>0x7d1b12be4780: fd fd fd fd fd[fd]fa fa fa fa fa fa fa fa fa fa
  0x7d1b12be4800: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7d1b12be4880: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7d1b12be4900: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7d1b12be4980: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7d1b12be4a00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
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
==562338==ABORTING
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

[4_~PPS_CommonLib_Slice_h_1957](https://github.com/sigdevel/pocs/blob/main/res/vvdec/vvdecapp/4/4_~PPS_CommonLib_Slice_h_1957)

#### Screenshots

![screen](https://github.com/sigdevel/pocs/blob/main/res/vvdec/vvdecapp/4/4_asan.png?raw=true "screen")

![screen](https://github.com/sigdevel/pocs/blob/main/res/vvdec/vvdecapp/4/4_vanilla.png?raw=true "screen")
