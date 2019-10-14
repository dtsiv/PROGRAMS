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

    //Команда отмены ожидания uint32_t, 0 вместо вспышки, 1 - полная отмена
    const uint32_t SNC_TYPE_WAIT_CANCEL = 0x9210;

    const uint32_t FLASH_TRIGGER = 0;
    const uint32_t FLASH_CANCEL = 1;

    struct SNC_STROBE
    {
        uint64_t execTime;   ///< Время выполнения (0 - сразу, если указано - время от первой вспышки)
        uint32_t strobeNo;   ///< Номер строба
        uint32_t flags;      ///< Флаги

        uint16_t pulsesCount;     ///< количество импульсов в стробе
        uint16_t pulseDuration;   ///< Длительность импульса
        uint16_t pulsePeriod;     ///< Период повторения импульсов
        uint16_t distance;        ///< Дистанция приема
        uint16_t blank;           ///< Бланк
        uint16_t padding;

        float azimuth;     ///< Азимут направления обзора
        float elevation;   ///< Угол места направления обзора

        uint32_t velocity;   ///< Скорость цели

        uint8_t pulseTimingNo;   ///< Номер времянки для обработки

        uint8_t panelSelect;   ///< Выбор грани для работы (юстировка)
        uint8_t txSelect;      ///< Выбор канала в грани (Юстировка)
        uint8_t ctrlChIndex;   ///< Номер контрольного канала (юстировка)
    };
}

namespace REG_OLD_VER1 {

    static const uint32_t FORMAT_VERSION        = 0x00000002;

}   // namespace REG_OLD_VER1

namespace REG {

    static const uint32_t HEADER_SIZE           = 5 * sizeof(uint32_t); // DTSIV: правильно 4 * sizeof(uint32_t)?
    static const char*    FILE_MAGIC            = "REGI";
    static const uint32_t FORMAT_VERSION        = 0x00000003;
    static const uint32_t DATA_CHUNKS_MAX_COUNT = 1000;

    static const uint32_t DATA_PTRS_MAX_COUNT_OFFSET = 8;
    static const uint32_t DATA_PTRS_COUNT_OFFSET     = 12;
    static const uint32_t DATA_POINTERS_OFFSET       = 16;
    static const uint32_t PROTOCOL_VERSION_OFFSET    = 20; // DTSIV: На этом месте должно быть смещение второй записи (строба)

}   // namespace REG

namespace CHR_TYPE {
    const uint32_t
    // Ps4-тип - Амплитудно фазовое распределение
    APHDISTR  = 100,
    VERSION_INFO = 0x11000,
    DSP_CONSOLE_DATA = 0x9999;
} // namespace CHR_TYPE

namespace CHR
{
    /// Флаги строба
    namespace SF {
     const uint32_t
        /// Флаг разрешения излучения
        EMI_ENABLE = SET_BIT(0),
        /// Флаг запроса АФР
        REQ_APHD = SET_BIT(1),
        /// Флаг проведения юстировки по этому АФР
        ADJ_SET = SET_BIT(2),
        /// Данный строб выполнять без юст. таблиц
        ADJ_IGNORE = SET_BIT(3),
        /// Флаг использования tx/panel select
        TX_SELECT = SET_BIT(4),
        /// Флаг использования rx/panel select
        RX_SELECT = SET_BIT(5),
        /// Измерение диаграммы, значения углов применить как отклонение луча
        DIAGRAM = SET_BIT(6),
        /// Этот строб - первый рабочий, ждем вспышки
        WAIT = SET_BIT(7),
        /// Этот строб - пустой, выполнить действия после строба
        NOEXEC = SET_BIT(8),
        /// Запросить текущий контроль после строба
        REQ_STATUS = SET_BIT(9),
        /// Включить ФКМ
        FKM_ENABLE = SET_BIT(10),
        /// Разрешить пилот-сигнал
        PILOT_ENABLE = SET_BIT(11),
        /// Разрешить ФКМ на пилот-сигнале
        PILOT_FKM = SET_BIT(12),
        /// Последний строб в пачке
        LAST_STROBE = SET_BIT(13);
    }

    struct STROBE_HEADER
    {
        uint64_t execTime;   ///< Время выполнения (0 - сразу, если указано - время от первой вспышки)
        uint32_t strobeNo;   ///< Номер строба
        uint32_t flags;      ///< Флаги

        uint16_t pCount;      ///< количество импульсов в стробе
        uint16_t pDuration;   ///< Длительность импульса
        uint16_t pPeriod;     ///< Период повторения импульсов
        uint16_t distance;    ///< Дистанция приема
        uint16_t blank;       ///< Бланк
        uint8_t  signalID;    ///< Номер модуляции сигнала
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

    // Управление каналами
    CHANCTL = 0x9201,

    // Управление положением антенны
    SET_ANTPOS     = 0x9002,
    CURRENT_ANTPOS = 0x9003,

    // Задание рабочей частоты, 4 байта float
    SET_FREQ = 0x9004,

    // Включение антенной решетки, uint32_t, 1 - включить, 0 - выключить
    SET_GRID_POWER = 0x9005,

    // Включить режим постоянного излучения
    SET_CONT_EMI = 0x9006,

    // Время прихода вспышки
    FLASH_TIME = 0x9205,

    // Команда отмены ожидания, тип uint32_t, 0 вместо вспышки, 1 - полная отмена
    WAIT_CANCEL = 0x9210,

    // Таблица юстировки
    ADJ_TBL = 0x9211,

    // Статус
    STATUS = 0x9250,

    // Задание амплитуды на передачу
    SET_TX_AMP = 0x9290,

    // Задание пауз до СИ и перед окончанием СП
    SET_STROBE_PAUSES = 0x9291,

    // Включение инверсии управления частотами
    SET_FREQ_INVERSION = 0x9292,

    // Провести переинициализацию ЦАП
    SEND_DAC_INIT = 0x9293,

    // Задать значения углов розочки лучей
    SET_ROSE_DELTA = 0x9294,

    // Постоянное вращение
    ROTATE_CONTINUOUS = 0x9295;
} // namespace SNC_TYPE

namespace SNC_DEBUG_TYPE {
/// Управление 8 регистром МЦД
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

        int16_t azimuth;     ///< Азимут направления обзора
        int16_t elevation;   ///< Угол места направления обзора

        uint32_t velocity;   ///< Скорость цели

        uint16_t pilotBlank;   ///< Пауза перед пилот сигналом
        uint16_t reserve;

        uint8_t pulseTimingNo;   ///< Номер времянки для обработки

        uint8_t panelSelect;   ///< Выбор грани для работы (юстировка)
        uint8_t rtxSelect;     ///< Выбор канала в грани (Юстировка)
        uint8_t ctrlChIndex;   ///< Номер контрольного канала (юстировка)
    };
    /**************************************************************************************************/

    /* Работа с положением антенны */
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

    // Флаги
    /// Это контрольный модуль
    const uint32_t CHANCTL_FLAG_CONTROL = 0x100;

    /// Структура управления каналами на прием/передачу
    struct CHANCTL
    {
        uint32_t moduleId;
        uint32_t flags;
        uint8_t  txMask[8];   ///< Маска каналов на передачу для каждого блинчика по 1 биту на канал
        uint8_t  rxMask[8];   ///< Маска каналов на прием для каждого блинчика по 1 биту на канал
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
