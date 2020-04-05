#ifndef QCHRPROTOCOMMON_H
#define QCHRPROTOCOMMON_H

#include <cstdint>
#include <cstddef>
#include <cstring>

#define SET_BIT(x) (1 << x)

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
        /// Абсолютное позиционирование луча
        ABSOLUTE_POS = SET_BIT(6),
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
        LAST_STROBE = SET_BIT(13),
        /// Разрешить поворот головы по угловым данным данного строба
        ROTATE_EN = SET_BIT(14);
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

#endif   // QCHRPROTOCOMMON_H
