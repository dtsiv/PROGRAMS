// !!! Modified: 2019.03.14
// 1. reserved[7] -> reserved[6]
// 2. flags introduced

// Modified: 2019.10.10
// File format version 3 introduced

// Modified: 2020.05.03
// Introduced latest changes 2020 from Tristan 
// (qchroprotoXX...X.h)

// Modified: 2020.07.22
// File format version 4 introduced

#ifndef QCHRPROTOACM_H
#define QCHRPROTOACM_H

#include "qchrprotocommon.h"
#include <cstdint>

namespace ACM_OLD_VER1 {

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

}

namespace REG_OLD_VER1 {

    static const uint32_t FORMAT_VERSION        = 0x00000002;

}   // namespace REG_OLD_VER1

namespace REG {

    static const uint32_t HEADER_SIZE           = 10 * sizeof(uint32_t); // currently, header=40 Bytes, QRegFileParser::sFileHdr
    static const char*    FILE_MAGIC            = "REGI";
    static const uint32_t FORMAT_VERSION        = 0x00000004;
    static const uint32_t DATA_CHUNKS_MAX_COUNT = 1000;

    static const uint32_t DATA_PTRS_MAX_COUNT_OFFSET = 8;
    static const uint32_t DATA_PTRS_COUNT_OFFSET     = 12;
    static const uint32_t DATA_POINTERS_OFFSET       = 16;
    static const uint32_t PROTOCOL_VERSION_OFFSET    = 20; // DTSIV: На этом месте должно быть смещение второй записи (строба)

}   // namespace REG



namespace ACM_TYPE {
    const uint32_t STROBE_DATA    = 0x8801,
                   STROBE         = 0x8802,
                   PROCSETTINGS   = 0x8803,
                   ROUNDCFG       = 0x8804,
                   RESET_CMD_FIFO = 0x8805,
                   ADD_DIST_REJECTION = 0x8806,
                   CLEAR_DIST_REJECTION = 0x8807,
                   STATUS         = 0x8850,
                   MCO_STATUS     = 0x8851;
} // namespace ACM_TYPE

namespace ACM {

    struct STROBE_DATA
    {
        CHR::STROBE_HEADER header;
        CHR::BEAM_POS beamPos;
        uint16_t inclBeta;
        uint16_t inclEpsilon;
        uint32_t beamCountsNum;
        uint16_t timeParams;
        uint16_t velocity;
        uint32_t reserved[3];
        //int16_t beam0Data[2][beamCountsNum];
        //int16_t beam1Data[2][beamCountsNum];
        //int16_t beam2Data[2][beamCountsNum];
        //int16_t beam3Data[2][beamCountsNum];
    };

    struct STROBE
    {
        CHR::STROBE_HEADER header;
        CHR::BEAM_POS beamPos;
        uint16_t detectionsCount;
        //DETECTION[detectionsCount];
    };

    struct DETECTION
    {
        uint16_t nV;
        uint16_t nD;
        uint16_t Amp;
        uint16_t SummA;
        int32_t  Re[4];
        int32_t  Im[4];
    };

    struct STATUS
    {
        uint32_t strobesRead;
        uint32_t cmdsRead;
        uint32_t lastStrobeRead;
        uint32_t strobesProcessed;
    };


    struct MCO_STATUS
    {
        uint16_t cmd;
        uint8_t  flags;
        uint8_t  address;
        uint16_t version;
        uint16_t reserved;
        uint16_t uart_err_count;
        uint16_t reset_count_recv0;
        uint16_t reset_count_recv1;
        uint16_t reset_count_recv2;
        uint16_t error_count_recv0;
        uint16_t error_count_recv1;
        uint16_t error_count_recv2;
        uint16_t temperature;
        uint16_t mcd_p_start;
        uint16_t ch_p_start;
        uint8_t  cnt_r_ch2;
        uint8_t  cnt_r_ch1;
        uint8_t  cnt_r_ch0;
        uint8_t  reserved2;
    };

    struct FACE_STATUS
    {
        uint8_t    face;
        uint8_t    flags;
        MCO_STATUS mcos[4];
    };


    struct PROCSETTINGS
    {
        uint16_t holdFactor;
        uint16_t holdSkip;
        uint16_t holdWindow;
        uint16_t fftSkipLeft;
        uint16_t fftSkipRight;
    };

}  // namespace ACM

#endif   // QCHRPROTOACM_H
