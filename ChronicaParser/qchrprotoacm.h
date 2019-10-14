// !!! Modified: 2019.03.14
// 1. reserved[7] -> reserved[6]
// 2. flags introduced

// Modified: 2019.10.10
// File format version 3 introduced

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

#define SET_BIT(x) (1 << x)

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

namespace SNC_OLD_VER1 {
    const uint32_t SNC_TYPE_STROBE = 0x9200;

    //������� ������ �������� uint32_t, 0 ������ �������, 1 - ������ ������
    const uint32_t SNC_TYPE_WAIT_CANCEL = 0x9210;

    const uint32_t FLASH_TRIGGER = 0;
    const uint32_t FLASH_CANCEL = 1;

    struct SNC_STROBE
    {
        uint64_t execTime;   ///< ����� ���������� (0 - �����, ���� ������� - ����� �� ������ �������)
        uint32_t strobeNo;   ///< ����� ������
        uint32_t flags;      ///< �����

        uint16_t pulsesCount;     ///< ���������� ��������� � ������
        uint16_t pulseDuration;   ///< ������������ ��������
        uint16_t pulsePeriod;     ///< ������ ���������� ���������
        uint16_t distance;        ///< ��������� ������
        uint16_t blank;           ///< �����
        uint16_t padding;

        float azimuth;     ///< ������ ����������� ������
        float elevation;   ///< ���� ����� ����������� ������

        uint32_t velocity;   ///< �������� ����

        uint8_t pulseTimingNo;   ///< ����� �������� ��� ���������

        uint8_t panelSelect;   ///< ����� ����� ��� ������ (���������)
        uint8_t txSelect;      ///< ����� ������ � ����� (���������)
        uint8_t ctrlChIndex;   ///< ����� ������������ ������ (���������)
    };
}

namespace REG_OLD_VER1 {

    static const uint32_t FORMAT_VERSION        = 0x00000002;

}   // namespace REG_OLD_VER1

namespace REG {

    static const uint32_t HEADER_SIZE           = 5 * sizeof(uint32_t); // DTSIV: ��������� 4 * sizeof(uint32_t)?
    static const char*    FILE_MAGIC            = "REGI";
    static const uint32_t FORMAT_VERSION        = 0x00000003;
    static const uint32_t DATA_CHUNKS_MAX_COUNT = 1000;

    static const uint32_t DATA_PTRS_MAX_COUNT_OFFSET = 8;
    static const uint32_t DATA_PTRS_COUNT_OFFSET     = 12;
    static const uint32_t DATA_POINTERS_OFFSET       = 16;
    static const uint32_t PROTOCOL_VERSION_OFFSET    = 20; // DTSIV: �� ���� ����� ������ ���� �������� ������ ������ (������)

}   // namespace REG

namespace CHR_TYPE {
    const uint32_t
    // Ps4-��� - ���������� ������� �������������
    APHDISTR  = 100,
    VERSION_INFO = 0x11000,
    DSP_CONSOLE_DATA = 0x9999;
} // namespace CHR_TYPE

namespace CHR
{
    /// ����� ������
    namespace SF {
     const uint32_t
        /// ���� ���������� ���������
        EMI_ENABLE = SET_BIT(0),
        /// ���� ������� ���
        REQ_APHD = SET_BIT(1),
        /// ���� ���������� ��������� �� ����� ���
        ADJ_SET = SET_BIT(2),
        /// ������ ����� ��������� ��� ���. ������
        ADJ_IGNORE = SET_BIT(3),
        /// ���� ������������� tx/panel select
        TX_SELECT = SET_BIT(4),
        /// ���� ������������� rx/panel select
        RX_SELECT = SET_BIT(5),
        /// ��������� ���������, �������� ����� ��������� ��� ���������� ����
        DIAGRAM = SET_BIT(6),
        /// ���� ����� - ������ �������, ���� �������
        WAIT = SET_BIT(7),
        /// ���� ����� - ������, ��������� �������� ����� ������
        NOEXEC = SET_BIT(8),
        /// ��������� ������� �������� ����� ������
        REQ_STATUS = SET_BIT(9),
        /// �������� ���
        FKM_ENABLE = SET_BIT(10),
        /// ��������� �����-������
        PILOT_ENABLE = SET_BIT(11),
        /// ��������� ��� �� �����-�������
        PILOT_FKM = SET_BIT(12),
        /// ��������� ����� � �����
        LAST_STROBE = SET_BIT(13);
    }

    struct STROBE_HEADER
    {
        uint64_t execTime;   ///< ����� ���������� (0 - �����, ���� ������� - ����� �� ������ �������)
        uint32_t strobeNo;   ///< ����� ������
        uint32_t flags;      ///< �����

        uint16_t pCount;      ///< ���������� ��������� � ������
        uint16_t pDuration;   ///< ������������ ��������
        uint16_t pPeriod;     ///< ������ ���������� ���������
        uint16_t distance;    ///< ��������� ������
        uint16_t blank;       ///< �����
        uint8_t  signalID;    ///< ����� ��������� �������
        uint8_t  padding;
    };
    struct BEAM_POS
    {
        int16_t sensorBeta;
        int16_t sensorEpsilon;
        int16_t beamBeta;
        int16_t beamEpsilon;
    };
} // namespace CHR

namespace SNC_TYPE {
    const uint32_t STROBE = 0x9200,

    // ���������� ��������
    CHANCTL = 0x9201,

    // ���������� ���������� �������
    SET_ANTPOS     = 0x9002,
    CURRENT_ANTPOS = 0x9003,

    // ������� ������� �������, 4 ����� float
    SET_FREQ = 0x9004,

    // ��������� �������� �������, uint32_t, 1 - ��������, 0 - ���������
    SET_GRID_POWER = 0x9005,

    // �������� ����� ����������� ���������
    SET_CONT_EMI = 0x9006,

    // ����� ������� �������
    FLASH_TIME = 0x9205,

    // ������� ������ ��������, ��� uint32_t, 0 ������ �������, 1 - ������ ������
    WAIT_CANCEL = 0x9210,

    // ������� ���������
    ADJ_TBL = 0x9211,

    // ������
    STATUS = 0x9250,

    // ������� ��������� �� ��������
    SET_TX_AMP = 0x9290,

    // ������� ���� �� �� � ����� ���������� ��
    SET_STROBE_PAUSES = 0x9291,

    // ��������� �������� ���������� ���������
    SET_FREQ_INVERSION = 0x9292,

    // �������� ����������������� ���
    SEND_DAC_INIT = 0x9293,

    // ������ �������� ����� ������� �����
    SET_ROSE_DELTA = 0x9294,

    // ���������� ��������
    ROTATE_CONTINUOUS = 0x9295;
} // namespace SNC_TYPE

namespace SNC_DEBUG_TYPE {
/// ���������� 8 ��������� ���
    const uint32_t MCDREG8 = 0x92B0,
    GRID_RAW_SEND = 0x92B0+1,
    SET_PLATFORM_AZ_ZERO = 0x92B0+2,
    SET_PLATFORM_EL_ZERO = 0x92B0+3,
    SET_PLATFORM_AZ_PID = 0x92B0+4,
    SET_PLATFORM_EL_PID = 0x92B0+5;

} // namespace SNC_DEBUG_TYPE

namespace SNC {

    // WAIT_CANCEL
    const uint32_t FLASH_TRIGGER = 0;
    const uint32_t FLASH_CANCEL  = 1;

    struct STROBE
    {
        ::CHR::STROBE_HEADER header;

        int16_t azimuth;     ///< ������ ����������� ������
        int16_t elevation;   ///< ���� ����� ����������� ������

        uint32_t velocity;   ///< �������� ����

        uint16_t pilotBlank;   ///< ����� ����� ����� ��������
        uint16_t reserve;

        uint8_t pulseTimingNo;   ///< ����� �������� ��� ���������

        uint8_t panelSelect;   ///< ����� ����� ��� ������ (���������)
        uint8_t rtxSelect;     ///< ����� ������ � ����� (���������)
        uint8_t ctrlChIndex;   ///< ����� ������������ ������ (���������)
    };
    /**************************************************************************************************/

    /* ������ � ���������� ������� */
    namespace APF {
     const uint32_t
        SET_POS = 0,
        STOP    = 1,
        SEEK    = 2;
    } // namespace APF

    struct ANTPOS
    {
        uint32_t flags;
        int16_t  azimuth;
        int16_t  elevation;
    };


    /**************************************************************************************************/

    // �����
    /// ��� ����������� ������
    const uint32_t CHANCTL_FLAG_CONTROL = 0x100;

    /// ��������� ���������� �������� �� �����/��������
    struct CHANCTL
    {
        uint32_t moduleId;
        uint32_t flags;
        uint8_t  txMask[8];   ///< ����� ������� �� �������� ��� ������� �������� �� 1 ���� �� �����
        uint8_t  rxMask[8];   ///< ����� ������� �� ����� ��� ������� �������� �� 1 ���� �� �����
    };


    /**************************************************************************************************/

    struct CONT_EMI
    {
        uint8_t enable;
        uint8_t channel;
    };

    /**************************************************************************************************/

    struct FLASH_TIME
    {
        uint64_t time;
        uint32_t number;
        uint32_t reserved;
    };

    /**************************************************************************************************/

    const uint32_t ADJ_FLAGS_RX = SET_BIT(1);
    const uint32_t ADJ_FLAGS_TX = SET_BIT(2);

    struct ADJ_TBL
    {
        uint32_t            moduleId;
        uint32_t            flags;
        // std::complex<float> adjTable[64];
    };

    /**************************************************************************************************/

    const uint16_t MRF_SET = SET_BIT(0);
    const uint16_t MRF_CLR = SET_BIT(1);

    struct MCDREG8CTL
    {
        uint16_t flags;
        uint16_t data;
        uint8_t  face;
        uint8_t  mcd;
        uint16_t padding;
    };

    struct GRIDDEBUGCTL
    {
        uint8_t face;
        uint8_t group;
        uint8_t device;
        uint8_t reg;
        uint16_t data;
    };

    /**************************************************************************************************/

    struct STATUS
    {
        uint16_t snc;
        uint16_t synth;
        uint16_t su;
        uint16_t auxCh;

        uint32_t strobesDone;
        uint32_t lastStrobeDone;
        uint32_t strobesDropped;
        uint32_t fpgaTemp;
    };

    namespace SU_FLAGS {
     const uint16_t OK = SET_BIT(0),
        AZ_ERR   = SET_BIT(1),
        EL_ERR   = SET_BIT(2),
        AZ_ROT   = SET_BIT(3),
        EL_ROT   = SET_BIT(4),
        GRID_PWR = SET_BIT(5),
        CMD_ERR  = SET_BIT(6),
        LINK_ERR = SET_BIT(8),
        LINK_TIM = SET_BIT(9);
    }

    namespace SYNTH_FLAGS {
        const uint16_t  PLL0     = SET_BIT(0),
        PLL1     = SET_BIT(1),
        LINK_ERR = SET_BIT(8);
    }

}   // namespace SNC

namespace ACM_TYPE {
    const uint32_t STROBE_DATA = 0x8801;
    const uint32_t STROBE = 0x8802;
    const uint32_t STATUS = 0x8850;
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
        uint16_t nFFT;
        uint16_t Amp;
    };

    struct STATUS
    {
        uint32_t strobesRead;
        uint32_t cmdsRead;
        uint32_t lastStrobeRead;
        uint32_t strobesProcessed;
    };


    struct CO_STATUS
    {
        uint8_t  face;
        uint8_t  co;
        uint16_t status[16];
    };

}  // namespace ACM



#endif   // QCHRPROTOACM_H
