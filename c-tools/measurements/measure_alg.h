
#ifndef MEASURE_ALG
#define MEASURE_ALG

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/*------------------------------------------------------------
  Битовые макросы
------------------------------------------------------------*/

#ifndef BIT_SET
#define BIT_SET(var, bit)       ((var) |= (bit))
#endif

#ifndef BIT_RESET
#define BIT_RESET(var, bit)     ((var) &= ~(bit))
#endif

#ifndef BIT_STATUS
#define BIT_STATUS(var, bit)    (((var) & (bit)) != 0)
#endif


/*------------------------------------------------------------
  Базовые параметры алгоритма
------------------------------------------------------------*/

#ifndef X_ALG_BUFF_LENGHT
#define X_ALG_BUFF_LENGHT           40u       /* Значение по умолчанию: 40. Максимальная физическая длина массива fBuff[]. */
#endif

#ifndef X_ALG_MIN_MES_BUF_RES
#define X_ALG_MIN_MES_BUF_RES       2u        /* Значение по умолчанию: 2. Минимальное число измерений до проверки критериев сброса буфера. */
#endif


/*------------------------------------------------------------
  Глобальные флаги компиляции функционала.
  0 — секция не компилируется.
  1 — секция компилируется, но может быть выключена
      у конкретного экземпляра через data->ulCalcFlags.
------------------------------------------------------------*/

#ifndef X_ALG_ENABLE_AVERAGE
#define X_ALG_ENABLE_AVERAGE              1
#endif

#ifndef X_ALG_ENABLE_SIGMA
#define X_ALG_ENABLE_SIGMA                1
#endif

#ifndef X_ALG_ENABLE_ERROR_ABS
#define X_ALG_ENABLE_ERROR_ABS            1
#endif

#ifndef X_ALG_ENABLE_ERROR_REL
#define X_ALG_ENABLE_ERROR_REL            1
#endif

#ifndef X_ALG_ENABLE_SLOPE_ABS
#define X_ALG_ENABLE_SLOPE_ABS            1
#endif

#ifndef X_ALG_ENABLE_SLOPE_REL
#define X_ALG_ENABLE_SLOPE_REL            1
#endif

#ifndef X_ALG_ENABLE_BUF_RES_DELTA
#define X_ALG_ENABLE_BUF_RES_DELTA        1
#endif

#ifndef X_ALG_ENABLE_BUF_RES_SLOPE_ABS
#define X_ALG_ENABLE_BUF_RES_SLOPE_ABS    1
#endif

#ifndef X_ALG_ENABLE_BUF_RES_RATIO_POS
#define X_ALG_ENABLE_BUF_RES_RATIO_POS    1
#endif

#ifndef X_ALG_ENABLE_BUF_RES_SLOPE_REL
#define X_ALG_ENABLE_BUF_RES_SLOPE_REL    1
#endif

#ifndef X_ALG_ENABLE_LPF1
#define X_ALG_ENABLE_LPF1                 1
#endif


/*------------------------------------------------------------
  Флаги состояния алгоритма

  В исходной версии алгоритма эти флаги задавались номерами битов:
  - X_ALG_FLAG_BUF_RES_FIRST = 0;
  - X_ALG_FLAG_BUF_RES       = 1;
  - X_ALG_FLAG_RESTART       = 2.

  В данной реализации макросы BIT_SET/BIT_RESET/BIT_STATUS работают
  с битовыми масками, поэтому ниже заданы маски соответствующих битов.
------------------------------------------------------------*/

#define X_ALG_FLAG_BUF_RES_FIRST    (1u << 0)  /* Исходное значение: 0. Маска: 0x01. Признак первого сброса буфера. */
#define X_ALG_FLAG_BUF_RES          (1u << 1)  /* Исходное значение: 1. Маска: 0x02. Признак необходимости сброса буфера. */
#define X_ALG_FLAG_RESTART          (1u << 2)  /* Исходное значение: 2. Маска: 0x04. Признак, что буфер был перезапущен. */


/*------------------------------------------------------------
  Флаги выбора расчетов для конкретного экземпляра data
------------------------------------------------------------*/

#define X_ALG_CALC_AVERAGE              (1UL << 0)
#define X_ALG_CALC_SIGMA                (1UL << 1)
#define X_ALG_CALC_ERROR_ABS            (1UL << 2)
#define X_ALG_CALC_ERROR_REL            (1UL << 3)
#define X_ALG_CALC_SLOPE_ABS            (1UL << 4)
#define X_ALG_CALC_SLOPE_REL            (1UL << 5)
#define X_ALG_CALC_BUF_RES_DELTA        (1UL << 6)
#define X_ALG_CALC_BUF_RES_SLOPE_ABS    (1UL << 7)
#define X_ALG_CALC_BUF_RES_RATIO_POS    (1UL << 8)
#define X_ALG_CALC_BUF_RES_SLOPE_REL    (1UL << 9)
#define X_ALG_CALC_LPF1                 (1UL << 10)




/*------------------------------------------------------------
  Производные флаги выполнения.
  Рассчитываются один раз из ulCalcFlags при инициализации
  или при явном изменении настроек.
------------------------------------------------------------*/

#define X_ALG_PROC_NEED_AVERAGE          (1UL << 0)
#define X_ALG_PROC_NEED_SIGMA            (1UL << 1)
#define X_ALG_PROC_NEED_SLOPE            (1UL << 2)
#define X_ALG_PROC_NEED_LPF1             (1UL << 3)

#define X_ALG_PROC_BUF_RES_DELTA         (1UL << 4)
#define X_ALG_PROC_BUF_RES_SLOPE_ABS     (1UL << 5)
#define X_ALG_PROC_BUF_RES_RATIO_POS     (1UL << 6)
#define X_ALG_PROC_BUF_RES_SLOPE_REL     (1UL << 7)


/*------------------------------------------------------------
  Флаги валидности результатов
------------------------------------------------------------*/

#define X_ALG_VALID_AVERAGE             (1UL << 0)
#define X_ALG_VALID_SIGMA               (1UL << 1)
#define X_ALG_VALID_ERROR_ABS           (1UL << 2)
#define X_ALG_VALID_ERROR_REL           (1UL << 3)
#define X_ALG_VALID_SLOPE_ABS           (1UL << 4)
#define X_ALG_VALID_SLOPE_REL           (1UL << 5)
#define X_ALG_VALID_LPF1                (1UL << 6)


/*------------------------------------------------------------
  Маска реально скомпилированного функционала
------------------------------------------------------------*/

#define X_ALG_COMPILED_CALC_FLAGS  ( \
  0UL \
  | ( X_ALG_ENABLE_AVERAGE             ? X_ALG_CALC_AVERAGE             : 0UL ) \
  | ( X_ALG_ENABLE_SIGMA               ? X_ALG_CALC_SIGMA               : 0UL ) \
  | ( X_ALG_ENABLE_ERROR_ABS           ? X_ALG_CALC_ERROR_ABS           : 0UL ) \
  | ( X_ALG_ENABLE_ERROR_REL           ? X_ALG_CALC_ERROR_REL           : 0UL ) \
  | ( X_ALG_ENABLE_SLOPE_ABS           ? X_ALG_CALC_SLOPE_ABS           : 0UL ) \
  | ( X_ALG_ENABLE_SLOPE_REL           ? X_ALG_CALC_SLOPE_REL           : 0UL ) \
  | ( X_ALG_ENABLE_BUF_RES_DELTA       ? X_ALG_CALC_BUF_RES_DELTA       : 0UL ) \
  | ( X_ALG_ENABLE_BUF_RES_SLOPE_ABS   ? X_ALG_CALC_BUF_RES_SLOPE_ABS   : 0UL ) \
  | ( X_ALG_ENABLE_BUF_RES_RATIO_POS   ? X_ALG_CALC_BUF_RES_RATIO_POS   : 0UL ) \
  | ( X_ALG_ENABLE_BUF_RES_SLOPE_REL   ? X_ALG_CALC_BUF_RES_SLOPE_REL   : 0UL ) \
  | ( X_ALG_ENABLE_LPF1                ? X_ALG_CALC_LPF1                : 0UL ) \
)

/*------------------------------------------------------------
  Наборы флагов по умолчанию и типовые наборы.
  X_ALG_CALC_LPF1 намеренно не включается в эти наборы.
------------------------------------------------------------*/

/*
  Универсальный сигнал:
  - может быть положительным, отрицательным, нулевым;
  - может менять знак;
  - может быть малым около нуля.

  Основной безопасный набор по умолчанию.
*/
#define X_ALG_CALC_SET_UNIVERSAL_REQUESTED \
  ( X_ALG_CALC_AVERAGE           | \
    X_ALG_CALC_SIGMA             | \
    X_ALG_CALC_ERROR_ABS         | \
    X_ALG_CALC_SLOPE_ABS         | \
    X_ALG_CALC_BUF_RES_DELTA     | \
    X_ALG_CALC_BUF_RES_SLOPE_ABS )

#define X_ALG_CALC_SET_UNIVERSAL \
  ( X_ALG_CALC_SET_UNIVERSAL_REQUESTED & X_ALG_COMPILED_CALC_FLAGS )


/*
  Знакопеременный сигнал:
  - сигнал может переходить через ноль;
  - относительные величины через среднее использовать нельзя.

  Совпадает с универсальным набором.
*/
#define X_ALG_CALC_SET_SIGNED_REQUESTED \
  X_ALG_CALC_SET_UNIVERSAL_REQUESTED

#define X_ALG_CALC_SET_SIGNED \
  X_ALG_CALC_SET_UNIVERSAL


/*
  Малый сигнал около нуля:
  - сигнал может быть положительным или отрицательным;
  - среднее может быть близко к нулю;
  - относительная ошибка и относительный наклон неинформативны.

  Совпадает с универсальным набором.
*/
#define X_ALG_CALC_SET_NEAR_ZERO_REQUESTED \
  X_ALG_CALC_SET_UNIVERSAL_REQUESTED

#define X_ALG_CALC_SET_NEAR_ZERO \
  X_ALG_CALC_SET_UNIVERSAL


/*
  Положительный сигнал, который может быть малым:
  - сигнал неотрицательный или положительный;
  - среднее может быть близко к нулю;
  - относительные критерии лучше не включать.

  Совпадает с универсальным набором.
*/
#define X_ALG_CALC_SET_POSITIVE_SMALL_REQUESTED \
  X_ALG_CALC_SET_UNIVERSAL_REQUESTED

#define X_ALG_CALC_SET_POSITIVE_SMALL \
  X_ALG_CALC_SET_UNIVERSAL


/*
  Строго положительный сигнал:
  - сигнал всегда > 0;
  - среднее значение не приближается к нулю;
  - относительные величины имеют физический смысл.

  Максимальный нормальный набор для положительного сигнала.
*/
#define X_ALG_CALC_SET_POSITIVE_REQUESTED \
  ( X_ALG_CALC_AVERAGE             | \
    X_ALG_CALC_SIGMA               | \
    X_ALG_CALC_ERROR_ABS           | \
    X_ALG_CALC_ERROR_REL           | \
    X_ALG_CALC_SLOPE_ABS           | \
    X_ALG_CALC_SLOPE_REL           | \
    X_ALG_CALC_BUF_RES_DELTA       | \
    X_ALG_CALC_BUF_RES_SLOPE_ABS   | \
    X_ALG_CALC_BUF_RES_RATIO_POS   | \
    X_ALG_CALC_BUF_RES_SLOPE_REL )

#define X_ALG_CALC_SET_POSITIVE \
  ( X_ALG_CALC_SET_POSITIVE_REQUESTED & X_ALG_COMPILED_CALC_FLAGS )


/*
  Сигнал, где нужен только расчёт статистики без автоматического сброса буфера.
*/
#define X_ALG_CALC_SET_STATS_ONLY_REQUESTED \
  ( X_ALG_CALC_AVERAGE   | \
    X_ALG_CALC_SIGMA     | \
    X_ALG_CALC_ERROR_ABS | \
    X_ALG_CALC_SLOPE_ABS )

#define X_ALG_CALC_SET_STATS_ONLY \
  ( X_ALG_CALC_SET_STATS_ONLY_REQUESTED & X_ALG_COMPILED_CALC_FLAGS )


/*
  Строго положительный сигнал, только статистика без сброса буфера.
*/
#define X_ALG_CALC_SET_POSITIVE_STATS_ONLY_REQUESTED \
  ( X_ALG_CALC_AVERAGE   | \
    X_ALG_CALC_SIGMA     | \
    X_ALG_CALC_ERROR_ABS | \
    X_ALG_CALC_ERROR_REL | \
    X_ALG_CALC_SLOPE_ABS | \
    X_ALG_CALC_SLOPE_REL )

#define X_ALG_CALC_SET_POSITIVE_STATS_ONLY \
  ( X_ALG_CALC_SET_POSITIVE_STATS_ONLY_REQUESTED & X_ALG_COMPILED_CALC_FLAGS )


/*
  Минимальный универсальный набор:
  - среднее;
  - СКО;
  - абсолютная ошибка среднего;
  - сброс по резкому отклонению.

  Без расчёта наклона.
*/
#define X_ALG_CALC_SET_BASIC_REQUESTED \
  ( X_ALG_CALC_AVERAGE       | \
    X_ALG_CALC_SIGMA         | \
    X_ALG_CALC_ERROR_ABS     | \
    X_ALG_CALC_BUF_RES_DELTA )

#define X_ALG_CALC_SET_BASIC \
  ( X_ALG_CALC_SET_BASIC_REQUESTED & X_ALG_COMPILED_CALC_FLAGS )


/*
  Только среднее.
*/
#define X_ALG_CALC_SET_AVERAGE_ONLY_REQUESTED \
  ( X_ALG_CALC_AVERAGE )

#define X_ALG_CALC_SET_AVERAGE_ONLY \
  ( X_ALG_CALC_SET_AVERAGE_ONLY_REQUESTED & X_ALG_COMPILED_CALC_FLAGS )


/*
  Набор по умолчанию.
  Совпадает с универсальным набором.
*/
#define X_ALG_CALC_DEFAULT_REQUESTED \
  X_ALG_CALC_SET_UNIVERSAL_REQUESTED

#define X_ALG_CALC_DEFAULT \
  X_ALG_CALC_SET_UNIVERSAL


/*
  Дополнительные режимы только для строго положительного сигнала.
  Используются как добавка к универсальному набору, если не нужен полный
  X_ALG_CALC_SET_POSITIVE.
*/
#define X_ALG_CALC_POSITIVE_SIGNAL_EXTRA_REQUESTED \
  ( X_ALG_CALC_ERROR_REL         | \
    X_ALG_CALC_SLOPE_REL         | \
    X_ALG_CALC_BUF_RES_RATIO_POS | \
    X_ALG_CALC_BUF_RES_SLOPE_REL )

#define X_ALG_CALC_POSITIVE_SIGNAL_EXTRA \
  ( X_ALG_CALC_POSITIVE_SIGNAL_EXTRA_REQUESTED & X_ALG_COMPILED_CALC_FLAGS )

/*------------------------------------------------------------
  Численные защиты и пороги.

  Макросы задают значения по умолчанию.
  Рабочие значения конкретного экземпляра хранятся в avr_settings_t.
------------------------------------------------------------*/

#ifndef X_ALG_AVG_EPS
#define X_ALG_AVG_EPS                   (1.0e-6f)  /* Значение по умолчанию: 1.0e-6. Минимальный модуль среднего для относительных расчетов. */
#endif

#ifndef X_ALG_INPUT_EPS
#define X_ALG_INPUT_EPS                 (1.0e-6f)  /* Значение по умолчанию: 1.0e-6. Минимальное входное значение для старого положительного критерия. */
#endif

#ifndef X_ALG_SIGMA_EPS
#define X_ALG_SIGMA_EPS                 (1.0e-6f)  /* Значение по умолчанию: 1.0e-6. Минимальное sigma, учитываемое в универсальном пороге сброса. */
#endif

#ifndef X_ALG_ABS_BUF_RES
#define X_ALG_ABS_BUF_RES               (50.0f)   /* Значение по умолчанию: 50.0. Абсолютная добавка к универсальному порогу сброса по отклонению. */
#endif

#ifndef X_ALG_REL_BUF_RES
#define X_ALG_REL_BUF_RES               (0.0f)    /* Значение по умолчанию: 0.0. Относительная часть универсального порога: fRelBufRes * scale. */
#endif

#ifndef X_ALG_SCALE_MIN
#define X_ALG_SCALE_MIN                 (1.0f)     /* Значение по умолчанию: 0.0. Минимальный масштаб для универсального относительного порога. */
#endif

#ifndef X_ALG_SIGMA_K_BUF_RES
#define X_ALG_SIGMA_K_BUF_RES           (4.0f)     /* Значение по умолчанию: 4.0. Множитель sigma в универсальном пороге сброса. */
#endif

#ifndef X_ALG_SLOPE_ABS_MAX
#define X_ALG_SLOPE_ABS_MAX             (50.01f)   /* Значение по умолчанию: 50.01. Порог абсолютного наклона, единиц сигнала на одно измерение. */
#endif

#ifndef X_ALG_KA1_MAX
#define X_ALG_KA1_MAX                   (2.0f)     /* Значение по умолчанию из исходного алгоритма: 2.0. Порог относительного наклона fKA1. */
#endif

#ifndef X_ALG_SLOPE_REL_MAX
#define X_ALG_SLOPE_REL_MAX             X_ALG_KA1_MAX  /* Значение по умолчанию: X_ALG_KA1_MAX = 2.0f. Порог относительного наклона fSlope/fAverageValue. */
#endif

#ifndef X_ALG_K1_BUF_RES
#define X_ALG_K1_BUF_RES                (150.0f)   /* Значение по умолчанию из исходного алгоритма: 150.0. Коэффициент K1 старого положительного критерия сброса. */
#endif

#ifndef X_ALG_K2_BUF_RES
#define X_ALG_K2_BUF_RES                (1.5f)     /* Значение по умолчанию из исходного алгоритма: 1.5. Коэффициент K2 старого положительного критерия сброса. */
#endif


/*------------------------------------------------------------
  Настройки одного экземпляра.
  Макросы X_ALG_* задают только значения по умолчанию.
  Конкретный экземпляр хранит рабочие значения в avr_settings_t.
------------------------------------------------------------*/

typedef struct {

  uint32_t      ulCalcFlags;       /* По умолчанию: X_ALG_CALC_DEFAULT. Набор расчетов X_ALG_CALC_* для экземпляра. */

  float         fLpf1Alpha;        /* По умолчанию: 0.1. Коэффициент ФНЧ 1 порядка, диапазон 0..1. */

  float         fAvgEps;           /* По умолчанию: X_ALG_AVG_EPS = 1.0e-6. Минимальный модуль среднего для относительных расчетов. */
  float         fInputEps;         /* По умолчанию: X_ALG_INPUT_EPS = 1.0e-6. Минимальное входное значение для старого положительного критерия. */
  float         fSigmaEps;         /* По умолчанию: X_ALG_SIGMA_EPS = 1.0e-6. Минимальное sigma, учитываемое в универсальном пороге. */

  float         fAbsBufRes;        /* По умолчанию: X_ALG_ABS_BUF_RES = 50.0. Абсолютная добавка к универсальному порогу сброса. */
  float         fRelBufRes;        /* По умолчанию: X_ALG_REL_BUF_RES = 0.0. Относительная часть универсального порога: fRelBufRes * scale. */
  float         fScaleMin;         /* По умолчанию: X_ALG_SCALE_MIN = 1.0. Минимальный масштаб для универсального относительного порога. */
  float         fSigmaKBufRes;     /* По умолчанию: X_ALG_SIGMA_K_BUF_RES = 4.0. Множитель sigma в универсальном пороге сброса. */

  float         fSlopeAbsMax;      /* По умолчанию: X_ALG_SLOPE_ABS_MAX = 50.0. Порог абсолютного наклона, единиц сигнала на одно измерение. */
  float         fSlopeRelMax;      /* По умолчанию: X_ALG_SLOPE_REL_MAX = X_ALG_KA1_MAX = 2.0. Порог относительного наклона fSlope/fAverageValue. */

  float         fK1BufRes;         /* По умолчанию: X_ALG_K1_BUF_RES = 150.0. Коэффициент K1 старого положительного критерия сброса. */
  float         fK2BufRes;         /* По умолчанию: X_ALG_K2_BUF_RES = 1.5. Коэффициент K2 старого положительного критерия сброса. */

} avr_settings_t;


/*------------------------------------------------------------
  Структура состояния.
  Состав структуры не зависит от X_ALG_ENABLE_*.
------------------------------------------------------------*/

typedef struct {
  
  uint8_t       ucResetBufUp;
  uint8_t       ucResetBufDown;
  uint8_t       ucResetBufKA1;
  
  float         fKA1;
  float         fSlope;
  
  uint16_t      ucMeasureNum;
  uint16_t      usBuffLength;
  
  uint8_t       ucFlags;

  uint32_t      ulCalcFlags;
  uint32_t      ulProcFlags;
  uint32_t      ulValidFlags;

  avr_settings_t sSettings;
  
  float         fAverageValue;
  float         fError;
  float         fMeanErrorAbs;
  float         fMeanErrorRel;
  float         fBuff[X_ALG_BUFF_LENGHT];
  float         sigma;

  float         fLpf1Value;
  float         fLpf1Alpha;
  uint8_t       ucLpf1Initialized;

  float         fLinN;
  float         fLinSumX;
  float         fLinDen;
  
} avr_data_t;


/*------------------------------------------------------------
  Публичные функции
------------------------------------------------------------*/

void vMeasureSettingsSetDefaults( avr_settings_t *settings );

void vMeasureSetSettings( avr_data_t *data, const avr_settings_t *settings );

void vMeasureInit( avr_data_t *data, const avr_settings_t *settings, uint16_t usBuffLength );

void vMeasureInitWithFlags( avr_data_t *data, uint32_t ulRequestedCalcFlags, uint16_t usBuffLength );

void vMeasureSetCalcFlags( avr_data_t *data, uint32_t ulRequestedCalcFlags );

void vMeasureSetLpf1Alpha( avr_data_t *data, float fAlpha );

void vMeasureProc( float fInputValue, avr_data_t *data );

void vAlgoritm ( float fInputValue, avr_data_t *data );
#ifdef __cplusplus
}
#endif

#endif /* MEASURE_ALG_INSTANCE_SETTINGS_V2_H */
