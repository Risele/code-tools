# README: алгоритм обработки измерений `measure_alg`

## 1. Назначение

Модуль `measure_alg` предназначен для обработки последовательности измерений типа `float`.

Он может выполнять несколько независимых операций:

- хранить последние измерения в буфере;
- считать среднее значение;
- считать стандартное отклонение;
- считать абсолютную и относительную ошибку среднего;
- считать наклон линейной аппроксимации буфера;
- выполнять сброс буфера при резком изменении сигнала;
- выполнять сброс буфера при устойчивом тренде;
- выполнять фильтрацию ФНЧ 1 порядка.

Код рассчитан на работу с разными типами сигналов:

- строго положительными;
- положительными, но малыми около нуля;
- нулевыми;
- отрицательными;
- знакопеременными;
- пуассоновскими.

Основной универсальный критерий сброса буфера не использует деление на среднее и не использует `sqrt(average)`, поэтому он применим к отрицательным и знакопеременным сигналам.

---

## 2. Файлы

Модуль состоит из двух файлов:

```c
measure_alg.h
measure_alg.c
```

### `measure_alg.h`

Содержит:

- глобальные флаги компиляции функционала;
- флаги расчётов `X_ALG_CALC_*`;
- готовые наборы флагов;
- значения настроек по умолчанию;
- структуру настроек `avr_settings_t`;
- структуру состояния `avr_data_t`;
- объявления публичных функций.

### `measure_alg.c`

Содержит реализацию:

- инициализации;
- установки настроек;
- обработки измерения;
- расчёта среднего;
- расчёта статистики;
- расчёта наклона;
- ФНЧ;
- критериев сброса буфера.

---

## 3. Основные структуры

### 3.1. `avr_data_t`

Основная структура состояния одного канала:

```c
avr_data_t data;
```

Если нужно обрабатывать несколько независимых сигналов, создаётся несколько экземпляров:

```c
avr_data_t ch1;
avr_data_t ch2;
avr_data_t ch3;
```

Каждый экземпляр имеет:

- свой буфер;
- свою активную длину буфера;
- свои настройки;
- свои флаги расчёта;
- свои результаты;
- своё состояние ФНЧ.

### 3.2. `avr_settings_t`

Структура настроек одного экземпляра:

```c
avr_settings_t settings;
```

Она содержит:

- `ulCalcFlags` — набор включённых расчётов;
- коэффициент ФНЧ;
- численные защиты;
- пороги сброса буфера;
- коэффициенты Пуассоновского положительного критерия.

Настройки хранятся внутри экземпляра:

```c
data.sSettings
```

Это значит, что разные экземпляры `avr_data_t` могут иметь разные пороги и разные наборы расчётов.

---

## 4. Максимальная и активная длина буфера

Максимальная длина буфера задаётся макросом:

```c
#define X_ALG_BUFF_LENGHT 40u
```

Внутри структуры всегда есть массив максимального размера:

```c
float fBuff[X_ALG_BUFF_LENGHT];
```

Но конкретный экземпляр может использовать только активную часть буфера. Активная длина задаётся при инициализации:

```c
vMeasureInit(&ch1, &settings1, 40u);
vMeasureInit(&ch2, &settings2, 10u);
```

В этом примере:

- `ch1` использует окно длиной 40;
- `ch2` использует окно длиной 10;
- физически массив в обоих случаях имеет размер `X_ALG_BUFF_LENGHT`.

Активная длина буфера задаётся только при инициализации и дальше вручную не меняется.

---

## 5. Два уровня настройки функционала

В коде есть два уровня настройки.

### 5.1. Глобальные флаги компиляции

Глобальные флаги определяют, какой функционал вообще попадёт в прошивку.

Примеры:

```c
#define X_ALG_ENABLE_AVERAGE              1
#define X_ALG_ENABLE_SIGMA                1
#define X_ALG_ENABLE_ERROR_REL            1
#define X_ALG_ENABLE_LPF1                 1
```

Если флаг равен `0`, соответствующая секция не компилируется.

Например:

```c
#define X_ALG_ENABLE_ERROR_REL 0
```

означает, что код расчёта относительной ошибки среднего не попадёт в прошивку.

Это полезно для микроконтроллеров, где важны размер прошивки и скорость выполнения.

### 5.2. Настройки конкретного экземпляра

Даже если функционал скомпилирован, конкретный экземпляр может его не использовать.

Для этого используется:

```c
settings.ulCalcFlags
```

или после инициализации:

```c
data.sSettings.ulCalcFlags
```

Пример:

```c
avr_settings_t settings1;
avr_settings_t settings2;

vMeasureSettingsSetDefaults(&settings1);
vMeasureSettingsSetDefaults(&settings2);

settings1.ulCalcFlags = X_ALG_CALC_SET_UNIVERSAL | X_ALG_CALC_LPF1;
settings2.ulCalcFlags = X_ALG_CALC_SET_AVERAGE_ONLY;

vMeasureInit(&ch1, &settings1, 40u);
vMeasureInit(&ch2, &settings2, 10u);
```

В этом примере:

- `ch1` использует универсальный набор и ФНЧ;
- `ch2` считает только среднее.

---

## 6. Производные флаги `ulProcFlags`

Поле:

```c
uint32_t ulProcFlags;
```

содержит внутренние производные флаги выполнения.

Оно рассчитывается автоматически в:

```c
vMeasureInit()
vMeasureSetSettings()
vMeasureSetCalcFlags()
```

Пользователь модуля не должен заполнять `ulProcFlags` вручную.

Смысл:

- `sSettings.ulCalcFlags` задаёт, что хочет пользователь;
- `ulProcFlags` заранее определяет, какие промежуточные расчёты реально нужны;
- `vMeasureProc()` не анализирует настройки каждый вызов, а использует уже подготовленные флаги.

---

## 7. Настройки по умолчанию

Макросы `X_ALG_*` в `measure_alg.h` задают значения по умолчанию. Рабочие значения конкретного экземпляра хранятся в `avr_settings_t`.

Заполнить структуру настроек значениями по умолчанию можно так:

```c
avr_settings_t settings;

vMeasureSettingsSetDefaults(&settings);
```

Текущие значения по умолчанию:

| Поле `avr_settings_t` | Значение по умолчанию | Назначение |
|---|---:|---|
| `ulCalcFlags` | `X_ALG_CALC_DEFAULT` | Набор расчётов экземпляра |
| `fLpf1Alpha` | `0.1f` | Коэффициент ФНЧ 1 порядка |
| `fAvgEps` | `1.0e-6f` | Минимальный модуль среднего для относительных расчётов |
| `fInputEps` | `1.0e-6f` | Минимальное входное значение для Пуассоновского положительного критерия |
| `fSigmaEps` | `1.0e-6f` | Минимальное `sigma`, учитываемое в универсальном пороге |
| `fAbsBufRes` | `50.01f` | Абсолютная добавка к универсальному порогу сброса |
| `fRelBufRes` | `50.0f` | Относительная часть универсального порога: `fRelBufRes * scale` |
| `fScaleMin` | `1.0f` | Минимальный масштаб для универсального относительного порога |
| `fSigmaKBufRes` | `4.0f` | Множитель `sigma` в универсальном пороге сброса |
| `fSlopeAbsMax` | `50.01f` | Порог абсолютного наклона, единиц сигнала на одно измерение |
| `fSlopeRelMax` | `2.0f` | Порог относительного наклона `fSlope / fAverageValue` |
| `fK1BufRes` | `150.0f` | Коэффициент `K1` Пуассоновского положительного критерия сброса |
| `fK2BufRes` | `1.5f` | Коэффициент `K2` Пуассоновского положительного критерия сброса |

Соответствующие макросы по умолчанию:

```c
#define X_ALG_AVG_EPS          (1.0e-6f)
#define X_ALG_INPUT_EPS        (1.0e-6f)
#define X_ALG_SIGMA_EPS        (1.0e-6f)
#define X_ALG_ABS_BUF_RES      (0.0f)
#define X_ALG_REL_BUF_RES      (50.0f)
#define X_ALG_SCALE_MIN        (1.0f)
#define X_ALG_SIGMA_K_BUF_RES  (4.0f)
#define X_ALG_SLOPE_ABS_MAX    (50.0f)
#define X_ALG_KA1_MAX          (2.0f)
#define X_ALG_SLOPE_REL_MAX    X_ALG_KA1_MAX
#define X_ALG_K1_BUF_RES       (150.0f)
#define X_ALG_K2_BUF_RES       (1.5f)
```

---

## 8. Инициализация

Перед первым использованием структуру обязательно нужно инициализировать.

Основной вариант:

```c
avr_data_t ch;
avr_settings_t settings;

vMeasureSettingsSetDefaults(&settings);
settings.ulCalcFlags = X_ALG_CALC_DEFAULT | X_ALG_CALC_LPF1;
settings.fLpf1Alpha = 0.1f;

vMeasureInit(&ch, &settings, 40u);
```

Сигнатура:

```c
void vMeasureInit(avr_data_t *data, const avr_settings_t *settings, uint16_t usBuffLength);
```

где:

- `data` — указатель на структуру состояния;
- `settings` — указатель на структуру настроек;
- `usBuffLength` — активная длина буфера.

Если `settings == 0`, используются настройки по умолчанию:

```c
vMeasureInit(&ch, 0, 40u);
```

Если `usBuffLength == 0`, будет использовано значение `1`.

Если `usBuffLength > X_ALG_BUFF_LENGHT`, будет использовано значение `X_ALG_BUFF_LENGHT`.

### Совместимый вариант инициализации только через флаги

Для упрощённого использования оставлена функция:

```c
void vMeasureInitWithFlags(avr_data_t *data, uint32_t ulRequestedCalcFlags, uint16_t usBuffLength);
```

Пример:

```c
vMeasureInitWithFlags(&ch, X_ALG_CALC_DEFAULT | X_ALG_CALC_LPF1, 40u);
```

Эта функция создаёт настройки по умолчанию, заменяет в них только `ulCalcFlags`, затем вызывает `vMeasureInit()`.

---

## 9. Изменение настроек во время работы

Настройки экземпляра можно изменить после инициализации.

### 9.1. Полная замена настроек

```c
avr_settings_t settings;

vMeasureSettingsSetDefaults(&settings);
settings.ulCalcFlags = X_ALG_CALC_SET_POSITIVE;
settings.fSlopeAbsMax = 100.0f;
settings.fAbsBufRes = 20.0f;
settings.fRelBufRes = 0.02f;
settings.fLpf1Alpha = 0.05f;

vMeasureSetSettings(&ch, &settings);
```

Функция:

```c
void vMeasureSetSettings(avr_data_t *data, const avr_settings_t *settings);
```

После вызова автоматически обновляются:

- `data->sSettings`;
- `data->ulCalcFlags`;
- `data->ulProcFlags`;
- кэшированное значение `data->fLpf1Alpha`.

Если `settings == 0`, будут загружены настройки по умолчанию.

### 9.2. Изменение только флагов расчёта

```c
vMeasureSetCalcFlags(&ch, X_ALG_CALC_SET_UNIVERSAL | X_ALG_CALC_LPF1);
```

Эта функция меняет:

```c
data.sSettings.ulCalcFlags
```

и пересчитывает `ulProcFlags`.

### 9.3. Изменение только коэффициента ФНЧ

```c
vMeasureSetLpf1Alpha(&ch, 0.1f);
```

Допустимый диапазон:

```c
0.0f <= alpha <= 1.0f
```

Значение автоматически ограничивается этим диапазоном.

---

## 10. Обработка нового измерения

Каждое новое измерение передаётся в функцию:

```c
vMeasureProc(fInputValue, &ch);
```

Пример:

```c
float value;

value = GetNewValue();
vMeasureProc(value, &ch);
```

После вызова `vMeasureProc()` результаты находятся в полях структуры `ch`.

---

## 11. Основные результаты

### 11.1. Среднее значение

```c
ch.fAverageValue
```

Валидность:

```c
if (ch.ulValidFlags & X_ALG_VALID_AVERAGE)
{
    UseAverage(ch.fAverageValue);
}
```

### 11.2. Стандартное отклонение

```c
ch.sigma
```

Валидность:

```c
if (ch.ulValidFlags & X_ALG_VALID_SIGMA)
{
    UseSigma(ch.sigma);
}
```

### 11.3. Абсолютная ошибка среднего

```c
ch.fMeanErrorAbs
```

Это абсолютная стандартная ошибка среднего.

Валидность:

```c
if (ch.ulValidFlags & X_ALG_VALID_ERROR_ABS)
{
    UseAbsError(ch.fMeanErrorAbs);
}
```

### 11.4. Относительная ошибка среднего

```c
ch.fMeanErrorRel
ch.fError
```

`fError` оставлено для совместимости со старым кодом.

Относительная ошибка имеет смысл только если среднее значение достаточно далеко от нуля. Если среднее около нуля, результат не считается валидным и остаётся равным `0`.

Валидность:

```c
if (ch.ulValidFlags & X_ALG_VALID_ERROR_REL)
{
    UseRelError(ch.fMeanErrorRel);
}
```

### 11.5. Абсолютный наклон

```c
ch.fSlope
```

Это наклон линейной аппроксимации значений в буфере в единицах сигнала на одно измерение.

Валидность:

```c
if (ch.ulValidFlags & X_ALG_VALID_SLOPE_ABS)
{
    UseSlope(ch.fSlope);
}
```

### 11.6. Относительный наклон

```c
ch.fKA1
```

Это отношение абсолютного наклона к среднему значению:

```text
fKA1 = fSlope / fAverageValue
```

Относительный наклон имеет смысл только если среднее значение не близко к нулю.

Валидность:

```c
if (ch.ulValidFlags & X_ALG_VALID_SLOPE_REL)
{
    UseRelSlope(ch.fKA1);
}
```

### 11.7. ФНЧ 1 порядка

```c
ch.fLpf1Value
```

Валидность:

```c
if (ch.ulValidFlags & X_ALG_VALID_LPF1)
{
    UseFilteredValue(ch.fLpf1Value);
}
```

ФНЧ рассчитывается по формуле:

```text
y = y + alpha * (x - y)
```

где:

- `x` — новое входное значение;
- `y` — предыдущее значение фильтра;
- `alpha` — коэффициент фильтра.

---

## 12. Сброс буфера

Алгоритм может автоматически сбросить буфер, если обнаружит изменение режима сигнала.

После сброса:

- буфер очищается;
- первое значение нового буфера становится равным текущему входному значению;
- статистика сбрасывается;
- устанавливается флаг `X_ALG_FLAG_RESTART`.

Проверка:

```c
if (ch.ucFlags & X_ALG_FLAG_RESTART)
{
    OnBufferRestart();
}
```

В текущей реализации `vMeasureProc()` сбрасывает `X_ALG_FLAG_RESTART` в начале каждого вызова и заново устанавливает его, если сброс произошёл в этом вызове. Поэтому флаг нужно читать после вызова `vMeasureProc()` и до следующего вызова.

---

## 13. Универсальный сброс по отклонению

Основной универсальный критерий сброса:

```c
X_ALG_CALC_BUF_RES_DELTA
```

Он использует выражение вида:

```text
|input - average| > threshold
```

Порог рассчитывается из трёх частей:

```text
threshold = absolute_part + relative_part + sigma_part
```

В текущей реализации рабочие значения берутся из настроек экземпляра:

```c
threshold = data->sSettings.fAbsBufRes
          + data->sSettings.fRelBufRes * scale
          + data->sSettings.fSigmaKBufRes * sigma;
```

где:

```c
scale = max(abs(average), data->sSettings.fScaleMin)
```

Этот критерий применим к:

- отрицательным значениям;
- нулевым значениям;
- малым сигналам;
- знакопеременным сигналам.

---

## 14. Сброс по наклону

Абсолютный сброс по наклону включается флагом:

```c
X_ALG_CALC_BUF_RES_SLOPE_ABS
```

Порог задаётся настройкой экземпляра:

```c
data->sSettings.fSlopeAbsMax
```

Условие:

```c
fabsf(fSlope) > data->sSettings.fSlopeAbsMax
```

Размерность `fSlopeAbsMax`:

```text
единиц сигнала / одно измерение
```

Относительный сброс по наклону включается флагом:

```c
X_ALG_CALC_BUF_RES_SLOPE_REL
```

Порог задаётся:

```c
data->sSettings.fSlopeRelMax
```

Условие:

```c
fabsf(fSlope / fAverageValue) > data->sSettings.fSlopeRelMax
```

Этот режим допустим только если среднее значение имеет устойчивый физический смысл как масштаб и не близко к нулю.

---

## 15. Пуассоновские относительные критерии

В коде оставлены оригинальные Пуассоновские относительные режимы:

```c
X_ALG_CALC_ERROR_REL
X_ALG_CALC_SLOPE_REL
X_ALG_CALC_BUF_RES_RATIO_POS
X_ALG_CALC_BUF_RES_SLOPE_REL
```

Они используют операции вида:

```c
input / average
slope / average
sqrt(average)
```

Поэтому они допустимы только для сигналов, которые физически строго положительны и не работают около нуля.

Пуассоновский критерий использует настройки:

```c
data->sSettings.fK1BufRes
data->sSettings.fK2BufRes
```

Порог:

```c
threshold = fK1BufRes / sqrt(average) + fK2BufRes;
```

Для универсального применения эти режимы лучше не включать.

---

## 16. Готовые наборы флагов

В `measure_alg.h` определены готовые наборы. `X_ALG_CALC_LPF1` намеренно не входит ни в один набор. Если ФНЧ нужен, его следует добавлять явно.

| Набор | Назначение |
|---|---|
| `X_ALG_CALC_SET_UNIVERSAL` | Безопасный универсальный набор для положительных, отрицательных, нулевых и знакопеременных сигналов |
| `X_ALG_CALC_SET_SIGNED` | Алиас универсального набора для знакопеременного сигнала |
| `X_ALG_CALC_SET_NEAR_ZERO` | Алиас универсального набора для сигналов около нуля |
| `X_ALG_CALC_SET_POSITIVE_SMALL` | Алиас универсального набора для положительного, но малого сигнала |
| `X_ALG_CALC_SET_POSITIVE` | Максимальный набор для строго положительного сигнала, не близкого к нулю |
| `X_ALG_CALC_SET_STATS_ONLY` | Только статистика без автоматического сброса буфера |
| `X_ALG_CALC_SET_POSITIVE_STATS_ONLY` | Статистика для строго положительного сигнала без сброса буфера |
| `X_ALG_CALC_SET_BASIC` | Среднее, СКО, абсолютная ошибка, сброс по резкому отклонению; без наклона |
| `X_ALG_CALC_SET_AVERAGE_ONLY` | Только среднее |
| `X_ALG_CALC_DEFAULT` | Алиас `X_ALG_CALC_SET_UNIVERSAL` |
| `X_ALG_CALC_POSITIVE_SIGNAL_EXTRA` | Дополнительные относительные режимы для строго положительного сигнала |

Пример:

```c
avr_settings_t settings;

vMeasureSettingsSetDefaults(&settings);
settings.ulCalcFlags = X_ALG_CALC_SET_UNIVERSAL | X_ALG_CALC_LPF1;

vMeasureInit(&ch, &settings, 40u);
```

---

## 17. Примеры настройки

### 17.1. Универсальный режим

```c
avr_data_t ch;
avr_settings_t settings;

vMeasureSettingsSetDefaults(&settings);
settings.ulCalcFlags = X_ALG_CALC_SET_UNIVERSAL;

vMeasureInit(&ch, &settings, 40u);
```

### 17.2. Универсальный режим плюс ФНЧ

```c
avr_data_t ch;
avr_settings_t settings;

vMeasureSettingsSetDefaults(&settings);
settings.ulCalcFlags = X_ALG_CALC_SET_UNIVERSAL | X_ALG_CALC_LPF1;
settings.fLpf1Alpha = 0.1f;

vMeasureInit(&ch, &settings, 40u);
```

### 17.3. Строго положительный сигнал со своими порогами

```c
avr_data_t ch;
avr_settings_t settings;

vMeasureSettingsSetDefaults(&settings);
settings.ulCalcFlags = X_ALG_CALC_SET_POSITIVE;
settings.fSlopeAbsMax = 100.0f;
settings.fSlopeRelMax = 2.0f;
settings.fK1BufRes = 150.0f;
settings.fK2BufRes = 1.5f;

vMeasureInit(&ch, &settings, 40u);
```

### 17.4. Два канала с разными порогами

```c
avr_data_t ch_fast;
avr_data_t ch_slow;
avr_settings_t fastSettings;
avr_settings_t slowSettings;

vMeasureSettingsSetDefaults(&fastSettings);
vMeasureSettingsSetDefaults(&slowSettings);

fastSettings.ulCalcFlags = X_ALG_CALC_SET_BASIC | X_ALG_CALC_LPF1;
fastSettings.fLpf1Alpha = 0.2f;
fastSettings.fAbsBufRes = 10.0f;
fastSettings.fSlopeAbsMax = 20.0f;

slowSettings.ulCalcFlags = X_ALG_CALC_SET_UNIVERSAL | X_ALG_CALC_LPF1;
slowSettings.fLpf1Alpha = 0.05f;
slowSettings.fAbsBufRes = 50.01f;
slowSettings.fSlopeAbsMax = 50.01f;

vMeasureInit(&ch_fast, &fastSettings, 10u);
vMeasureInit(&ch_slow, &slowSettings, 40u);
```

### 17.5. Упрощённая инициализация через флаги

```c
avr_data_t ch;

vMeasureInitWithFlags(&ch, X_ALG_CALC_SET_AVERAGE_ONLY, 10u);
```

---

## 18. Как отключить ненужный функционал из прошивки

Перед подключением `measure_alg.h` можно задать глобальные флаги:

```c
#define X_ALG_ENABLE_ERROR_REL            0
#define X_ALG_ENABLE_BUF_RES_RATIO_POS    0
#define X_ALG_ENABLE_SLOPE_REL            0
```

Так код относительных режимов не будет скомпилирован.

Для проверки лучше смотреть `.map`-файл или листинг сборки.

Для GCC обычно полезны опции:

```text
-Os
-ffunction-sections
-fdata-sections
-Wl,--gc-sections
```

---

## 19. Типовой цикл использования

```c
#include "measure_alg.h"

avr_data_t channel;

void Init(void)
{
    avr_settings_t settings;

    vMeasureSettingsSetDefaults(&settings);
    settings.ulCalcFlags = X_ALG_CALC_DEFAULT | X_ALG_CALC_LPF1;
    settings.fLpf1Alpha = 0.1f;

    vMeasureInit(&channel, &settings, 40u);
}

void Loop(void)
{
    float input;

    input = GetInputValue();

    vMeasureProc(input, &channel);

    if (channel.ulValidFlags & X_ALG_VALID_AVERAGE)
    {
        UseAverage(channel.fAverageValue);
    }

    if (channel.ulValidFlags & X_ALG_VALID_LPF1)
    {
        UseFilteredValue(channel.fLpf1Value);
    }

    if (channel.ucFlags & X_ALG_FLAG_RESTART)
    {
        OnBufferRestart();
    }
}
```

---

## 20. Матрица применимости режимов

Обозначения:

- `●` — можно включать;
- `○` — можно включать, но только если нужен соответствующий результат;
- `△` — можно включать только при специальных условиях;
- `×` — не рекомендуется включать;
- `—` — не относится к данному случаю.

| Режим / флаг | Малый сигнал около нуля | Отрицательный сигнал | Знакопеременный сигнал | Строго положительный сигнал | Если нужен только результат без сброса | Если нужен сброс буфера |
|---|---:|---:|---:|---:|---:|---:|
| `X_ALG_CALC_AVERAGE` | ● | ● | ● | ● | ● | ○ |
| `X_ALG_CALC_SIGMA` | ● | ● | ● | ● | ● | ○ |
| `X_ALG_CALC_ERROR_ABS` | ● | ● | ● | ● | ● | ○ |
| `X_ALG_CALC_ERROR_REL` | × | △ | △ | ● | ○ | — |
| `X_ALG_CALC_SLOPE_ABS` | ● | ● | ● | ● | ● | ○ |
| `X_ALG_CALC_SLOPE_REL` | × | △ | △ | ● | ○ | — |
| `X_ALG_CALC_BUF_RES_DELTA` | ● | ● | ● | ● | — | ● |
| `X_ALG_CALC_BUF_RES_SLOPE_ABS` | ● | ● | ● | ● | — | ● |
| `X_ALG_CALC_BUF_RES_RATIO_POS` | × | × | × | △ | — | △ |
| `X_ALG_CALC_BUF_RES_SLOPE_REL` | × | △ | △ | △ | — | △ |
| `X_ALG_CALC_LPF1` | ● | ● | ● | ● | ● | — |

---

## 21. Матрица выходных параметров и флагов `X_ALG_CALC_*`

Обозначения:

- `●` — флаг непосредственно включает расчёт выходного параметра;
- `○` — параметр рассчитывается как зависимость для работы флага;
- `—` — не используется;
- `*` — значение может быть установлено косвенно при срабатывании критерия сброса.

| Выходное значение / флаг | `AVERAGE` | `SIGMA` | `ERROR_ABS` | `ERROR_REL` | `SLOPE_ABS` | `SLOPE_REL` | `BUF_RES_DELTA` | `BUF_RES_SLOPE_ABS` | `BUF_RES_RATIO_POS` | `BUF_RES_SLOPE_REL` | `LPF1` |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| `fAverageValue` | ● | ○ | ○ | ○ | — | ○ | ○ | — | ○ | ○ | — |
| `sigma` | — | ● | ○ | ○ | — | — | ○ | — | — | — | — |
| `fMeanErrorAbs` | — | — | ● | — | — | — | — | — | — | — | — |
| `fMeanErrorRel` | — | — | — | ● | — | — | — | — | — | — | — |
| `fError` | — | — | — | ● | — | — | — | — | — | — | — |
| `fSlope` | — | — | — | — | ● | — | — | ○ | — | ○ | — |
| `fKA1` | — | — | — | — | — | ● | — | — | — | ○ | — |
| `fLpf1Value` | — | — | — | — | — | — | — | — | — | — | ● |
| `ulValidFlags` | ● | ● | ● | ● | ● | ● | ● | ● | ● | ● | ● |
| `ucResetBufUp` | — | — | — | — | — | — | * | — | * | — | — |
| `ucResetBufDown` | — | — | — | — | — | — | * | — | * | — | — |
| `ucResetBufKA1` | — | — | — | — | — | — | — | * | — | * | — |
| `ucFlags: X_ALG_FLAG_BUF_RES` | — | — | — | — | — | — | * | * | * | * | — |
| `ucFlags: X_ALG_FLAG_RESTART` | — | — | — | — | — | — | * | * | * | * | — |

---

## 22. Что важно помнить

1. Всегда вызывайте `vMeasureInit()` или `vMeasureInitWithFlags()` перед первым использованием.
2. Не меняйте `data.usBuffLength` вручную после инициализации.
3. Для малых, нулевых, отрицательных и знакопеременных сигналов используйте универсальные режимы.
4. Относительные режимы используйте только для строго положительных сигналов, не близких к нулю.
5. Перед использованием результата проверяйте `data.ulValidFlags`.
6. Если расчёт отключён или результат не может быть вычислен, соответствующее поле остаётся равным `0`.
7. Если нужно изменить пороги для конкретного канала, меняйте `avr_settings_t` и вызывайте `vMeasureSetSettings()`.
