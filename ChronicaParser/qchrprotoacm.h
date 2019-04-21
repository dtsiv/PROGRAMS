// !!! Modified: 2019.03.14
// 1. reserved[7] -> reserved[6]
// 2. flags introduced

#ifndef QCHRPROTOACM_H
#define QCHRPROTOACM_H

#ifdef __TI_COMPILER_VERSION__
#    include <stdint.h>
#    define DSP_BIOS
#else
#  if 0
#    ifdef _MSC_VER
#        pragma warning(push)
#        pragma warning(disable : 4200)
#        include <pstdint.h>
#    else
#        ifdef __cplusplus
#            include <cstdint>
#        else
#            include <stdint.h>
#        endif
#    endif
#  endif
#endif

// #include "stdint.h"
#include <cstdint>

const uint32_t ACM_TYPE_STROBE_DATA = 0x8801;

struct ACM_STROBE_DATA
{
    uint64_t execTime;
    uint32_t strobeNo;
    uint32_t flags; // since 03.2019: use flags
    uint16_t sensorBeta;
    uint16_t sensorEpsilon;
    float    beamBeta;
    float    beamEpsilon;
    uint16_t inclBeta;
    uint16_t inclEpsilon;
    uint32_t beamCountsNum;
    uint16_t timeParams;
    uint16_t velocity;
    uint32_t reserved[6];
    //uint32_t reserved[7];

    //int16_t beam0Data[2][beamCountsNum];
    //int16_t beam1Data[2][beamCountsNum];
    //int16_t beam2Data[2][beamCountsNum];
    //int16_t beam3Data[2][beamCountsNum];
};

#endif   // QCHRPROTOACM_H
