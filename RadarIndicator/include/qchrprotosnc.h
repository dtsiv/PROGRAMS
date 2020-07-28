#ifndef QCHRPROTOSNC_H
#define QCHRPROTOSNC_H
// Created: 2020.05.03
// Used for SNC-related constants and data structures

#include "qchrprotocommon.h"
#include <complex>

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
    ROTATE_CONTINUOUS = 0x9295,

    // Настройки датчиков положения
    SET_SU_SETTINGS = 0x9296,

    // Задание амплитуды на передачу
    SET_RX_AMP = 0x9297,

    // Образ сигнала
    SIGNAL_IMAGE = 0x9220;

} // namespace SNC_TYPE

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
        std::complex<float> adjTable[64];
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
        LINK_TIM = SET_BIT(9),
        NO_SU    = SET_BIT(15);
    }

    namespace SYNTH_FLAGS {
        const uint16_t  PLL0     = SET_BIT(0),
        PLL1     = SET_BIT(1),
        LINK_ERR = SET_BIT(8);
    }

    struct SIGNAL_IMAGE {
        uint32_t sigID;
        uint8_t  signal[16 * 8];
    };

    // clang-format off
    namespace SU_SET_FLAGS {
        const uint32_t HAS_SU = SET_BIT(0),
        HAS_BETA_SENSOR    = SET_BIT(1),
        HAS_EPSILON_SENSOR = SET_BIT(2);
    }
    // clang-format on

    struct SU_SETTINGS {
        uint32_t flags;
        int16_t  beta;
        int16_t  epsilon;
    };

    struct PLATFORM_PID {
        uint32_t pidID;
        float    p;
        float    i;
        float    d;
    };

}   // namespace SNC

namespace SNC_DEBUG_TYPE {
/// Управление 8 регистром МЦД
    const uint32_t MCDREG8 = 0x92B0,
    GRID_RAW_SEND = 0x92B0+1,
    SET_PLATFORM_AZ_ZERO = 0x92B0+2,
    SET_PLATFORM_EL_ZERO = 0x92B0+3,
    SET_PLATFORM_PID     = 0x92B0+4;
} // namespace SNC_DEBUG_TYPE

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

#endif   // QCHRPROTOSNC_H

