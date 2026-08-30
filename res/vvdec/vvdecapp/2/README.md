e493ce51f13a2dea72cd58354652ed4e0f509a0e

vvdecapp version 3.3.0-dev

https://github.com/fraunhoferhhi/vvdec/issues

NOTE: findings #1 and #2 share the same root cause (APS object lifetime vs concurrent slice tasks: `ParameterSetMap::storePS` frees a `shared_ptr<APS>` on re-definition of an in-use `aps_id` while a worker thread still reads it). This finding (#2) is the read in `CABACReader::readAlf` (DecoderLib/CABACReader.cpp:430); #1 is a second read site in `AdaptiveLoopFilter::reconstructCoeff` (CommonLib/AdaptiveLoopFilter.cpp:894). Reported separately (kept as distinct crash sites), not merged.
