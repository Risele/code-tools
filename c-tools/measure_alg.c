/*
  measure_alg_instance_settings_v2.c
  Реализация версии с настройками экземпляра.
*/

#include "measure_alg_instance_settings_v2.h"

#include <string.h>
#include <math.h>


/*------------------------------------------------------------
  Внутренние функции
------------------------------------------------------------*/

static inline float fMeasureLimitMin( float value, float minValue )
{
  if ( value < minValue )
    return minValue;

  return value;
}


static inline void vMeasureSanitizeSettings( avr_settings_t *settings )
{
  settings->ulCalcFlags &= X_ALG_COMPILED_CALC_FLAGS;

  settings->fLpf1Alpha = fMeasureLimitMin( settings->fLpf1Alpha, 0.0f );
  if ( settings->fLpf1Alpha > 1.0f )
    settings->fLpf1Alpha = 1.0f;

  settings->fAvgEps = fMeasureLimitMin( settings->fAvgEps, 0.0f );
  settings->fInputEps = fMeasureLimitMin( settings->fInputEps, 0.0f );
  settings->fSigmaEps = fMeasureLimitMin( settings->fSigmaEps, 0.0f );

  settings->fAbsBufRes = fMeasureLimitMin( settings->fAbsBufRes, 0.0f );
  settings->fRelBufRes = fMeasureLimitMin( settings->fRelBufRes, 0.0f );
  settings->fScaleMin = fMeasureLimitMin( settings->fScaleMin, 0.0f );
  settings->fSigmaKBufRes = fMeasureLimitMin( settings->fSigmaKBufRes, 0.0f );

  settings->fSlopeAbsMax = fMeasureLimitMin( settings->fSlopeAbsMax, 0.0f );
  settings->fSlopeRelMax = fMeasureLimitMin( settings->fSlopeRelMax, 0.0f );

  settings->fK1BufRes = fMeasureLimitMin( settings->fK1BufRes, 0.0f );
  settings->fK2BufRes = fMeasureLimitMin( settings->fK2BufRes, 0.0f );
}


static inline void vMeasureSetupLinCoeff( avr_data_t *data )
{
  uint64_t n64;
  uint64_t sumX64;
  uint64_t sumX264;
  uint64_t den64;

  n64 = (uint64_t)data->usBuffLength;

  if ( n64 < 2u )
  {
    data->fLinN = (float)n64;
    data->fLinSumX = 0.0f;
    data->fLinDen = 0.0f;
    return;
  }

  sumX64 = n64 * ( n64 + 1u ) / 2u;
  sumX264 = n64 * ( n64 + 1u ) * ( 2u * n64 + 1u ) / 6u;
  den64 = n64 * sumX264 - sumX64 * sumX64;

  data->fLinN = (float)n64;
  data->fLinSumX = (float)sumX64;
  data->fLinDen = (float)den64;
}


static inline void vMeasureSetupProcFlags( avr_data_t *data )
{
  uint32_t procFlags;

  data->sSettings.ulCalcFlags &= X_ALG_COMPILED_CALC_FLAGS;
  data->ulCalcFlags = data->sSettings.ulCalcFlags;

  procFlags = 0UL;

#if X_ALG_ENABLE_AVERAGE
  if ( data->ulCalcFlags & X_ALG_CALC_AVERAGE )
    procFlags |= X_ALG_PROC_NEED_AVERAGE;
#endif

#if X_ALG_ENABLE_SIGMA
  if ( data->ulCalcFlags & X_ALG_CALC_SIGMA )
  {
    procFlags |= X_ALG_PROC_NEED_AVERAGE;
    procFlags |= X_ALG_PROC_NEED_SIGMA;
  }
#endif

#if X_ALG_ENABLE_ERROR_ABS
  if ( data->ulCalcFlags & X_ALG_CALC_ERROR_ABS )
  {
    procFlags |= X_ALG_PROC_NEED_AVERAGE;
    procFlags |= X_ALG_PROC_NEED_SIGMA;
  }
#endif

#if X_ALG_ENABLE_ERROR_REL
  if ( data->ulCalcFlags & X_ALG_CALC_ERROR_REL )
  {
    procFlags |= X_ALG_PROC_NEED_AVERAGE;
    procFlags |= X_ALG_PROC_NEED_SIGMA;
  }
#endif

#if X_ALG_ENABLE_SLOPE_ABS
  if ( data->ulCalcFlags & X_ALG_CALC_SLOPE_ABS )
    procFlags |= X_ALG_PROC_NEED_SLOPE;
#endif

#if X_ALG_ENABLE_SLOPE_REL
  if ( data->ulCalcFlags & X_ALG_CALC_SLOPE_REL )
  {
    procFlags |= X_ALG_PROC_NEED_AVERAGE;
    procFlags |= X_ALG_PROC_NEED_SLOPE;
  }
#endif

#if X_ALG_ENABLE_BUF_RES_DELTA
  if ( data->ulCalcFlags & X_ALG_CALC_BUF_RES_DELTA )
  {
    procFlags |= X_ALG_PROC_NEED_AVERAGE;
    procFlags |= X_ALG_PROC_NEED_SIGMA;
    procFlags |= X_ALG_PROC_BUF_RES_DELTA;
  }
#endif

#if X_ALG_ENABLE_BUF_RES_SLOPE_ABS
  if ( data->ulCalcFlags & X_ALG_CALC_BUF_RES_SLOPE_ABS )
  {
    procFlags |= X_ALG_PROC_NEED_SLOPE;
    procFlags |= X_ALG_PROC_BUF_RES_SLOPE_ABS;
  }
#endif

#if X_ALG_ENABLE_BUF_RES_RATIO_POS
  if ( data->ulCalcFlags & X_ALG_CALC_BUF_RES_RATIO_POS )
  {
    procFlags |= X_ALG_PROC_NEED_AVERAGE;
    procFlags |= X_ALG_PROC_BUF_RES_RATIO_POS;
  }
#endif

#if X_ALG_ENABLE_BUF_RES_SLOPE_REL
  if ( data->ulCalcFlags & X_ALG_CALC_BUF_RES_SLOPE_REL )
  {
    procFlags |= X_ALG_PROC_NEED_AVERAGE;
    procFlags |= X_ALG_PROC_NEED_SLOPE;
    procFlags |= X_ALG_PROC_BUF_RES_SLOPE_REL;
  }
#endif

#if X_ALG_ENABLE_LPF1
  if ( data->ulCalcFlags & X_ALG_CALC_LPF1 )
    procFlags |= X_ALG_PROC_NEED_LPF1;
#endif

  data->ulProcFlags = procFlags;
}


static inline void vMeasureSyncSettingsCache( avr_data_t *data )
{
  data->ulCalcFlags = data->sSettings.ulCalcFlags & X_ALG_COMPILED_CALC_FLAGS;
  data->sSettings.ulCalcFlags = data->ulCalcFlags;

  data->fLpf1Alpha = data->sSettings.fLpf1Alpha;
}


static inline void vMeasureResetOutputs( avr_data_t *data )
{
  data->ulValidFlags = 0u;

  data->fKA1 = 0.0f;
  data->fSlope = 0.0f;
  data->fError = 0.0f;
  data->fMeanErrorAbs = 0.0f;
  data->fMeanErrorRel = 0.0f;
  data->sigma = 0.0f;

  if ( ( data->ulProcFlags & X_ALG_PROC_NEED_AVERAGE ) == 0u )
    data->fAverageValue = 0.0f;

  if ( ( data->ulProcFlags & X_ALG_PROC_NEED_LPF1 ) == 0u )
  {
    data->fLpf1Value = 0.0f;
    data->ucLpf1Initialized = 0u;
  }
}


static inline void vMeasureUpdateBuffer( float fInputValue, avr_data_t *data )
{
  uint16_t i;

  if ( data->ucMeasureNum >= data->usBuffLength )
  {
    if ( data->usBuffLength > 1u )
    {
      for ( i = 0u ; i < data->usBuffLength - 1u ; i++ )
      {
        data->fBuff[i] = data->fBuff[i + 1u];
      }
    }

    data->fBuff[data->usBuffLength - 1u] = fInputValue;
    data->ucMeasureNum = data->usBuffLength;
  }
  else
  {
    if ( BIT_STATUS( data->ucFlags, X_ALG_FLAG_BUF_RES_FIRST ) )
    {
      data->ucMeasureNum = 0u;
      BIT_RESET( data->ucFlags, X_ALG_FLAG_BUF_RES_FIRST );
    }

    data->fBuff[data->ucMeasureNum] = fInputValue;
    data->ucMeasureNum++;
  }
}


static inline void vMeasureCalcAverage( avr_data_t *data )
{
  uint16_t i;
  float tmp;

  if ( ( data->ulProcFlags & X_ALG_PROC_NEED_AVERAGE ) == 0u )
  {
    data->fAverageValue = 0.0f;
    return;
  }

  if ( data->ucMeasureNum == 0u )
  {
    data->fAverageValue = 0.0f;
    return;
  }

  tmp = 0.0f;

  for ( i = 0u; i < data->ucMeasureNum; i++ ) 
  {
    tmp += data->fBuff[i];
  } 

  tmp /= (float)data->ucMeasureNum;

  data->fAverageValue = tmp;
  data->ulValidFlags |= X_ALG_VALID_AVERAGE;
}


static inline void vMeasureCalcSigmaAndErrors( avr_data_t *data )
{
#if X_ALG_ENABLE_SIGMA || \
    X_ALG_ENABLE_ERROR_ABS || \
    X_ALG_ENABLE_ERROR_REL || \
    X_ALG_ENABLE_BUF_RES_DELTA

  uint16_t i;
  float a;
  float s;
  float s1;

  if ( ( data->ulProcFlags & X_ALG_PROC_NEED_SIGMA ) == 0u )
    return;

  if ( ( data->ucMeasureNum <= 1u ) ||
       ( ( data->ulValidFlags & X_ALG_VALID_AVERAGE ) == 0u ) )
  {
    return;
  }

  s = 0.0f;

  for ( i = 0u; i < data->ucMeasureNum; i++ ) 
  {
    a = data->fBuff[i] - data->fAverageValue;	                        
    s += a * a; 
  } 

  s1 = s / (float)data->ucMeasureNum / (float)( data->ucMeasureNum - 1u );
  s = s / (float)( data->ucMeasureNum - 1u );

  data->sigma = sqrtf(s);
  data->ulValidFlags |= X_ALG_VALID_SIGMA;

#if X_ALG_ENABLE_ERROR_ABS
  if ( data->ulCalcFlags & X_ALG_CALC_ERROR_ABS )
  {
    data->fMeanErrorAbs = sqrtf(s1);
    data->ulValidFlags |= X_ALG_VALID_ERROR_ABS;
  }
#endif

#if X_ALG_ENABLE_ERROR_REL
  if ( data->ulCalcFlags & X_ALG_CALC_ERROR_REL )
  {
    if ( fabsf(data->fAverageValue) > data->sSettings.fAvgEps )
    {
      data->fMeanErrorRel = sqrtf(s1) / fabsf(data->fAverageValue) * 100.0f;
      data->fError = data->fMeanErrorRel;
      data->ulValidFlags |= X_ALG_VALID_ERROR_REL;
    }
    else
    {
      data->fMeanErrorRel = 0.0f;
      data->fError = 0.0f;
    }
  }
#endif

#else

  (void)data;

#endif
}


static inline void vMeasureCalcSlopeAndCheckReset( avr_data_t *data )
{
#if X_ALG_ENABLE_SLOPE_ABS || \
    X_ALG_ENABLE_SLOPE_REL || \
    X_ALG_ENABLE_BUF_RES_SLOPE_ABS || \
    X_ALG_ENABLE_BUF_RES_SLOPE_REL

  uint16_t i;
  float fS2;
  float fS3;
  float fSlope;
  float fSlopeRel;

  if ( ( data->ulProcFlags & X_ALG_PROC_NEED_SLOPE ) == 0u )
    return;

  if ( data->ucMeasureNum < data->usBuffLength )
    return;

  if ( data->usBuffLength <= 1u )
    return;

  if ( data->fLinDen <= 0.0f )
    return;

  fS2 = 0.0f;
  fS3 = 0.0f;

  for ( i = 0u ; i < data->usBuffLength ; i++ )
  {
    fS2 += data->fBuff[i];
    fS3 += data->fBuff[i] * (float)(i + 1u);
  } 

  fSlope = ( data->fLinN * fS3 - data->fLinSumX * fS2 ) / data->fLinDen;

#if X_ALG_ENABLE_SLOPE_ABS
  if ( data->ulCalcFlags & X_ALG_CALC_SLOPE_ABS )
  {
    data->fSlope = fSlope;
    data->ulValidFlags |= X_ALG_VALID_SLOPE_ABS;
  }
#endif

#if X_ALG_ENABLE_BUF_RES_SLOPE_ABS
  if ( data->ulProcFlags & X_ALG_PROC_BUF_RES_SLOPE_ABS )
  {
    if ( fabsf(fSlope) > data->sSettings.fSlopeAbsMax )
    {
      BIT_SET( data->ucFlags, X_ALG_FLAG_BUF_RES );
      data->ucResetBufKA1++;
    }
  }
#endif

#if X_ALG_ENABLE_SLOPE_REL || X_ALG_ENABLE_BUF_RES_SLOPE_REL
  fSlopeRel = 0.0f;

  if ( ( data->ulValidFlags & X_ALG_VALID_AVERAGE ) &&
       ( fabsf(data->fAverageValue) > data->sSettings.fAvgEps ) )
  {
    fSlopeRel = fSlope / data->fAverageValue;

#if X_ALG_ENABLE_SLOPE_REL
    if ( data->ulCalcFlags & X_ALG_CALC_SLOPE_REL )
    {
      data->fKA1 = fSlopeRel;
      data->ulValidFlags |= X_ALG_VALID_SLOPE_REL;
    }
#endif

#if X_ALG_ENABLE_BUF_RES_SLOPE_REL
    if ( data->ulProcFlags & X_ALG_PROC_BUF_RES_SLOPE_REL )
    {
      data->fKA1 = fSlopeRel;
      data->ulValidFlags |= X_ALG_VALID_SLOPE_REL;

      if ( fabsf(fSlopeRel) > data->sSettings.fSlopeRelMax )
      {
        BIT_SET( data->ucFlags, X_ALG_FLAG_BUF_RES );
        data->ucResetBufKA1++;
      }
    }
#endif
  }
  else
  {
    data->fKA1 = 0.0f;
  }
#endif

#else

  (void)data;

#endif
}


static inline void vMeasureCalcLpf1( float fInputValue, avr_data_t *data )
{
#if X_ALG_ENABLE_LPF1

  if ( ( data->ulProcFlags & X_ALG_PROC_NEED_LPF1 ) == 0u )
  {
    data->fLpf1Value = 0.0f;
    data->ucLpf1Initialized = 0u;
    return;
  }

  if ( data->fLpf1Alpha < 0.0f )
    data->fLpf1Alpha = 0.0f;

  if ( data->fLpf1Alpha > 1.0f )
    data->fLpf1Alpha = 1.0f;

  if ( data->ucLpf1Initialized == 0u )
  {
    data->fLpf1Value = fInputValue;
    data->ucLpf1Initialized = 1u;
  }
  else
  {
    data->fLpf1Value = data->fLpf1Value + data->fLpf1Alpha * ( fInputValue - data->fLpf1Value );
  }

  data->ulValidFlags |= X_ALG_VALID_LPF1;

#else

  data->fLpf1Value = 0.0f;
  data->ucLpf1Initialized = 0u;

  (void)fInputValue;

#endif
}


static inline void vMeasureCheckDeltaReset( float fInputValue, avr_data_t *data )
{
#if X_ALG_ENABLE_BUF_RES_DELTA

  float delta;
  float scale;
  float threshold;

  if ( ( data->ulProcFlags & X_ALG_PROC_BUF_RES_DELTA ) == 0u )
    return;

  if ( data->ucMeasureNum < X_ALG_MIN_MES_BUF_RES )
    return;

  if ( ( data->ulValidFlags & X_ALG_VALID_AVERAGE ) == 0u )
    return;

  delta = fabsf( fInputValue - data->fAverageValue );

  scale = fabsf( data->fAverageValue );
  if ( scale < data->sSettings.fScaleMin )
    scale = data->sSettings.fScaleMin;

  threshold = data->sSettings.fAbsBufRes + data->sSettings.fRelBufRes * scale;

  if ( data->ulValidFlags & X_ALG_VALID_SIGMA )
  {
    if ( data->sigma > data->sSettings.fSigmaEps )
      threshold += data->sSettings.fSigmaKBufRes * data->sigma;
  }

  if ( delta > threshold )
  {
    if ( fInputValue > data->fAverageValue )
      data->ucResetBufUp++;
    else
      data->ucResetBufDown++;

    BIT_SET( data->ucFlags, X_ALG_FLAG_BUF_RES_FIRST );
    BIT_SET( data->ucFlags, X_ALG_FLAG_BUF_RES );
  }

#else

  (void)fInputValue;
  (void)data;

#endif
}


static inline void vMeasureCheckPositiveRatioReset( float fInputValue, avr_data_t *data )
{
#if X_ALG_ENABLE_BUF_RES_RATIO_POS

  double avg;
  double input;
  double threshold;

  if ( ( data->ulProcFlags & X_ALG_PROC_BUF_RES_RATIO_POS ) == 0u )
    return;

  if ( data->ucMeasureNum < X_ALG_MIN_MES_BUF_RES )
    return;

  if ( ( data->ulValidFlags & X_ALG_VALID_AVERAGE ) == 0u )
    return;

  if ( data->fAverageValue <= data->sSettings.fAvgEps )
    return;

  if ( fInputValue <= data->sSettings.fInputEps )
    return;

  avg = (double)data->fAverageValue;
  input = (double)fInputValue;

  threshold = data->sSettings.fK1BufRes / sqrt(avg) + data->sSettings.fK2BufRes;

  if ( input / avg > threshold )
  {
    data->ucResetBufUp++;
    BIT_SET( data->ucFlags, X_ALG_FLAG_BUF_RES_FIRST );
    BIT_SET( data->ucFlags, X_ALG_FLAG_BUF_RES );
  }
  else if ( avg / input > threshold )
  {
    data->ucResetBufDown++;
    BIT_SET( data->ucFlags, X_ALG_FLAG_BUF_RES_FIRST );
    BIT_SET( data->ucFlags, X_ALG_FLAG_BUF_RES );
  }

#else

  (void)fInputValue;
  (void)data;

#endif
}


static inline void vMeasureApplyBufferReset( float fInputValue, avr_data_t *data )
{
  if ( BIT_STATUS( data->ucFlags, X_ALG_FLAG_BUF_RES ) == 0u )
    return;

  memset( data->fBuff, 0, sizeof(data->fBuff) );

  data->ucMeasureNum = 1u;
  data->fBuff[0] = fInputValue;

  data->fAverageValue = 0.0f;
  data->sigma = 0.0f;
  data->fMeanErrorAbs = 0.0f;
  data->fMeanErrorRel = 0.0f;
  data->fError = 0.0f;
  data->fSlope = 0.0f;
  data->fKA1 = 0.0f;

  data->ulValidFlags = 0u;

  if ( data->ulProcFlags & X_ALG_PROC_NEED_AVERAGE )
  {
    data->fAverageValue = fInputValue;
    data->ulValidFlags |= X_ALG_VALID_AVERAGE;
  }

#if X_ALG_ENABLE_LPF1
  if ( data->ulProcFlags & X_ALG_PROC_NEED_LPF1 )
  {
    data->fLpf1Value = fInputValue;
    data->ucLpf1Initialized = 1u;
    data->ulValidFlags |= X_ALG_VALID_LPF1;
  }
  else
  {
    data->fLpf1Value = 0.0f;
    data->ucLpf1Initialized = 0u;
  }
#else
  data->fLpf1Value = 0.0f;
  data->ucLpf1Initialized = 0u;
#endif

  BIT_RESET( data->ucFlags, X_ALG_FLAG_BUF_RES );
  BIT_RESET( data->ucFlags, X_ALG_FLAG_BUF_RES_FIRST );
  BIT_SET( data->ucFlags, X_ALG_FLAG_RESTART );
}


/*------------------------------------------------------------
  Публичные функции
------------------------------------------------------------*/

void vMeasureSettingsSetDefaults( avr_settings_t *settings )
{
  settings->ulCalcFlags = X_ALG_CALC_DEFAULT;

  settings->fLpf1Alpha = 0.1f;

  settings->fAvgEps = X_ALG_AVG_EPS;
  settings->fInputEps = X_ALG_INPUT_EPS;
  settings->fSigmaEps = X_ALG_SIGMA_EPS;

  settings->fAbsBufRes = X_ALG_ABS_BUF_RES;
  settings->fRelBufRes = X_ALG_REL_BUF_RES;
  settings->fScaleMin = X_ALG_SCALE_MIN;
  settings->fSigmaKBufRes = X_ALG_SIGMA_K_BUF_RES;

  settings->fSlopeAbsMax = X_ALG_SLOPE_ABS_MAX;
  settings->fSlopeRelMax = X_ALG_SLOPE_REL_MAX;

  settings->fK1BufRes = X_ALG_K1_BUF_RES;
  settings->fK2BufRes = X_ALG_K2_BUF_RES;

  vMeasureSanitizeSettings( settings );
}


void vMeasureSetSettings( avr_data_t *data, const avr_settings_t *settings )
{
  if ( settings == 0 )
  {
    vMeasureSettingsSetDefaults( &data->sSettings );
  }
  else
  {
    data->sSettings = *settings;
  }

  vMeasureSanitizeSettings( &data->sSettings );
  vMeasureSyncSettingsCache( data );
  vMeasureSetupProcFlags( data );

#if X_ALG_ENABLE_LPF1
  if ( ( data->ulProcFlags & X_ALG_PROC_NEED_LPF1 ) == 0u )
  {
    data->fLpf1Value = 0.0f;
    data->ucLpf1Initialized = 0u;
  }
#else
  data->fLpf1Value = 0.0f;
  data->ucLpf1Initialized = 0u;
#endif
}


void vMeasureInit( avr_data_t *data, const avr_settings_t *settings, uint16_t usBuffLength )
{
  memset( data, 0, sizeof(*data) );

  if ( settings == 0 )
  {
    vMeasureSettingsSetDefaults( &data->sSettings );
  }
  else
  {
    data->sSettings = *settings;
  }

  vMeasureSanitizeSettings( &data->sSettings );
  vMeasureSyncSettingsCache( data );

  if ( usBuffLength == 0u )
    usBuffLength = 1u;

  if ( usBuffLength > X_ALG_BUFF_LENGHT )
    usBuffLength = X_ALG_BUFF_LENGHT;

  data->usBuffLength = usBuffLength;

  vMeasureSetupLinCoeff( data );
  vMeasureSetupProcFlags( data );
}


void vMeasureInitWithFlags( avr_data_t *data, uint32_t ulRequestedCalcFlags, uint16_t usBuffLength )
{
  avr_settings_t settings;

  vMeasureSettingsSetDefaults( &settings );
  settings.ulCalcFlags = ulRequestedCalcFlags;

  vMeasureInit( data, &settings, usBuffLength );
}


void vMeasureSetCalcFlags( avr_data_t *data, uint32_t ulRequestedCalcFlags )
{
  data->sSettings.ulCalcFlags = ulRequestedCalcFlags & X_ALG_COMPILED_CALC_FLAGS;

  vMeasureSanitizeSettings( &data->sSettings );
  vMeasureSyncSettingsCache( data );
  vMeasureSetupProcFlags( data );

#if X_ALG_ENABLE_LPF1
  if ( ( data->ulProcFlags & X_ALG_PROC_NEED_LPF1 ) == 0u )
  {
    data->fLpf1Value = 0.0f;
    data->ucLpf1Initialized = 0u;
  }
#else
  data->fLpf1Value = 0.0f;
  data->ucLpf1Initialized = 0u;
#endif
}


void vMeasureSetLpf1Alpha( avr_data_t *data, float fAlpha )
{
  if ( fAlpha < 0.0f )
    fAlpha = 0.0f;

  if ( fAlpha > 1.0f )
    fAlpha = 1.0f;

  data->sSettings.fLpf1Alpha = fAlpha;
  data->fLpf1Alpha = fAlpha;
}


void vMeasureProc( float fInputValue, avr_data_t *data )
{

    
  BIT_RESET( data->ucFlags, X_ALG_FLAG_RESTART );
  
  vMeasureResetOutputs( data );

  vMeasureUpdateBuffer( fInputValue, data );

  vMeasureCalcAverage( data );

  vMeasureCalcSigmaAndErrors( data );

  vMeasureCalcSlopeAndCheckReset( data );

  vMeasureCalcLpf1( fInputValue, data );

  vMeasureCheckDeltaReset( fInputValue, data );

  vMeasureCheckPositiveRatioReset( fInputValue, data );

  vMeasureApplyBufferReset( fInputValue, data );
}


inline void vAlgoritm( float fInputValue, avr_data_t *data  )
{
  //Обертка для совместимости. По сути, ничего не делает, как и в оригинальном коде
  if ( BIT_STATUS( data->ucFlags, X_ALG_FLAG_RESTART ) )
  {
   //...
  }			                                                        // новое измерение если сброс статистики (сработал один из критериев сброса буфера
  
  if( data->ucMeasureNum < X_ALG_BUFF_LENGHT)
  {
   //...
  }
  else
  {
    //...
  }
   
   vMeasureProc( fInputValue, data );
}
 