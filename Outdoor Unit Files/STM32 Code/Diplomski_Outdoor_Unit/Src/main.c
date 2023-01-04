/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "myStructs.h"
#include "glassLCD.h"
#include "SHT21.h"
#include "rtc.h"
#include "sleep.h"
#include "BMP180.h"
#include "Si1147.h"
#include "RF24.h"
#include "communication.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NO_DEEP_SLEEP   0
#define DEEP_SLEEP      1
#define BMP180_ERROR    1
#define SHT21_ERROR     2
#define SI1147_ERROR    3
#define NRF24_ERROR     4

#define ALL_MEASUREMENTS        0b111111111
#define TEMP_MEASUREMENT        0b000000001
#define HUMIDITY_MEASUREMENT    0b000000010
#define PRESSURE_MEASUREMENT    0b000000100
#define UV_MEASUREMENT          0b000001000
#define LIGHT_MEASUREMENT       0b000010000
#define SOLAR_MEASUREMENT       0b000100000
#define WINDSPEED_MEASUREMENT   0b001000000
#define WINDDIR_MEASUREMENT     0b010000000
#define BATTERY_MEASUREMENT     0b100000000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
struct syncStructHandle syncStruct = {SYNC_HEADER};
struct measruementHandle weatherData;
struct measruementHandle currentWeatherData;
const char lcdTest[] = {"88888888"};
const char errStr[] = {"ERR-%03d"};
const char* windStr[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};
volatile uint32_t interruptButton = 0;
volatile uint8_t alarmInterruptFlag = 0;
const uint16_t measurementTable[] = { TEMP_MEASUREMENT | HUMIDITY_MEASUREMENT, PRESSURE_MEASUREMENT, UV_MEASUREMENT | LIGHT_MEASUREMENT, SOLAR_MEASUREMENT, WINDSPEED_MEASUREMENT, WINDDIR_MEASUREMENT, BATTERY_MEASUREMENT};
uint32_t sendInterval;
int16_t windDirCalibration = 0;
char lcdTemp[20];
uint8_t firstTimeSync = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */
void writeError(uint8_t _e, uint8_t _forceSleep);
void readWeatherData(struct measruementHandle *_w, uint16_t _flags);
void getSolarData(double *_energyJ, double *_energy);
uint32_t getADC(uint32_t _ch);
float getWindSpeed();
int16_t getWindDir(uint32_t _pin, int16_t _offset);
float getWindSpeed();
float getBatteryVoltage();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_ADC_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  // Init LCD Driver
  glassLCD_Begin();

  // Test LCD (and wait for debugger to connect)
  glassLCD_WriteData((char*) lcdTest);
  glassLCD_SetDot(0b11111111);
  glassLCD_WriteArrow(0b11111111);
  glassLCD_Update();
  HAL_Delay(2000);

  // Dummy ADC readings to calibrate ADC and remove ranks
  getADC(ADC_CHANNEL_0);
  getADC(ADC_CHANNEL_1);
  getADC(ADC_CHANNEL_4);

  // Init Pressure & Temp sensor
  if (!BMP180_Init()) writeError(BMP180_ERROR, DEEP_SLEEP);

  // Init Humidity & Temp sensor
  if (!SHT21_Init()) writeError(SHT21_ERROR, DEEP_SLEEP);

  // Init Si1147 Sensor for ALS and UV
  if (!Si1147_Init()) writeError(SI1147_ERROR, DEEP_SLEEP);

  RF24_init(NRF24_CE_GPIO_Port, NRF24_CE_Pin, NRF24_CSN_GPIO_Port, NRF24_CSN_Pin);
  if (!RF24_begin()) writeError(NRF24_ERROR, DEEP_SLEEP);

  // Enable UV meas.
  Si1147_SetUV();

  // Set time on RTC
  RTC_SetTime(1609459200);

  // Setup RF communication (speed, RF Channel, RF Power, rtc)
  communication_Setup();

  // Wait for sync signal from indoor station
  uint8_t syncTimeout = 180;
  uint8_t rxBuffer[32] = {0};
  uint8_t syncSuccess = 0;
  while (!syncSuccess && syncTimeout > 0)
  {
    if (communication_Transmit(&syncStruct, sizeof(syncStruct), rxBuffer))
    {
      if (rxBuffer[0] == SYNC_HEADER)
      {
        memcpy(&syncStruct, rxBuffer, sizeof(syncStruct));
        syncSuccess = 1;
      }
    }
    char lcdTemp[9];
    sprintf(lcdTemp, "SYNC %3d", syncTimeout--);
    glassLCD_WriteData(lcdTemp);
    glassLCD_Update();
    HAL_Delay(1000);
  }
  // Flush all unsent data and power down the module
  RF24_flush_rx();
  RF24_flush_tx();
  RF24_powerDown();

  if (syncSuccess)
  {
    struct tm hTime;

    RTC_SetTime(syncStruct.myEpoch);
    RTC_SetAlarmEpoch(syncStruct.sendEpoch, RTC_ALARMMASK_DATEWEEKDAY);
    sendInterval = syncStruct.sendEpoch - syncStruct.myEpoch;
    firstTimeSync = 1;

    glassLCD_WriteData("SYNC OK");
    glassLCD_Update();
    HAL_Delay(1000);

    hTime = RTC_EpochToHuman(syncStruct.myEpoch);
    sprintf(lcdTemp, " %2d%02d%02d", hTime.tm_hour, hTime.tm_min, hTime.tm_sec);
    glassLCD_WriteData(lcdTemp);
    glassLCD_SetDot(0b00101000);
    glassLCD_Update();
    HAL_Delay(1000);
  }
  else
  {
    sendInterval = 300;
    RTC_SetAlarmEpoch(RTC_GetTime() + sendInterval, RTC_ALARMMASK_DATEWEEKDAY);
    glassLCD_WriteData("NO SYNC");
    glassLCD_Update();
    HAL_Delay(1000);
  }

  // Get North direction for wind direction calibration
  for (int i = 0; i < 5; i++)
  {
    sprintf(lcdTemp, "D CAL %d", 5 - i);
    glassLCD_WriteData(lcdTemp);
    glassLCD_Update();
    windDirCalibration += getWindDir(ADC_CHANNEL_1, 0);
    HAL_Delay(1000);
  }
  windDirCalibration /= 5;
  readWeatherData(&currentWeatherData, ALL_MEASUREMENTS);

  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t k = 0;
  while (1)
  {
    uint8_t lcdDot = 0;
    uint8_t lcdArrow = 0;
    if (k == 0)
    {
      int16_t t = round(currentWeatherData.tempSHT * 10);
      int16_t h = round(currentWeatherData.humidity * ((currentWeatherData.humidity >= 100) ? 1 : 10));
      lcdDot = (h >= 1000) ? 0b00100000 : 0b00100010;
      sprintf(lcdTemp, "%3d%01d %2d%01d", t / 10, abs(t % 10), abs(h / 10), abs(h % 10));
      lcdDot = (currentWeatherData.humidity >= 100) ? 0b00100000 : 0b00100010;
      lcdArrow = 0b10000000;
    }
    if (k == 1)
    {
      uint16_t p = round(currentWeatherData.pressure * 10);
      sprintf(lcdTemp, "%4d%1d", abs(p / 10), abs(p % 10));
      lcdDot = 0b00010000;
      lcdArrow = 0b01000000;
    }
    if (k == 2)
    {
      int16_t uv = (currentWeatherData.uv * 10);
      int16_t vis = currentWeatherData.light;
      sprintf(lcdTemp, "%2d%1d %4d", abs(uv / 10), abs(uv % 10), vis);
      lcdDot = 0b01000000;
      lcdArrow = 0b00100000;
    }
    if (k == 3)
    {
      int energyJ = round(currentWeatherData.solarJ * 10);
      sprintf(lcdTemp, "%2d%1d %4d", abs(energyJ / 10), abs(energyJ % 10), (int) (currentWeatherData.solarW));
      lcdDot = 0b01000000;
      lcdArrow = 0b00010000;
    }
    if (k == 4)
    {
      int wind = round(currentWeatherData.windSpeed * 10);
      sprintf(lcdTemp, "%3d%1d", abs(wind / 10), abs(wind % 10));
      lcdDot = 0b00100000;
      lcdArrow = 0b00001000;
    }
    if (k == 5)
    {
      sprintf(lcdTemp, "%3s %3d", windStr[(int) ((currentWeatherData.windDir / 22.5) + 0.5) % 16], currentWeatherData.windDir);
      lcdArrow = 0b00000100;
    }
    if (k == 6)
    {
      uint16_t batt = round((currentWeatherData.battery) * 100);
      struct tm t = RTC_EpochToHuman(RTC_GetTime());
      sprintf(lcdTemp, "%2d%02d %1d%02d", t.tm_hour, t.tm_min, batt / 100, abs(batt % 100));
      lcdArrow = 0b00000010;
      lcdDot = 0b01000100;
    }
    glassLCD_WriteData(lcdTemp);
    glassLCD_SetDot(lcdDot);
    glassLCD_WriteArrow(lcdArrow);
    glassLCD_Update();
    Sleep_LightSleep();

    if (alarmInterruptFlag == 1)
    {
      alarmInterruptFlag = 0;
      readWeatherData(&weatherData, ALL_MEASUREMENTS);
      currentWeatherData = weatherData;
      uint8_t dataSent = 0;

      if (firstTimeSync)
      {
        RF24_powerUp();
        communication_Setup();
        uint8_t rxBuffer[32] = {0};
        uint8_t sendTimeout = 25;
        struct data1StructHandle data1 = {DATA1_HEADER};
        struct data2StructHandle data2 = {DATA2_HEADER};
        void* dataStructList[2] = {&data1, &data2};
        size_t dataStructListSize[2] = {sizeof(data1), sizeof(data2)};
        data1.uv = weatherData.uv * 100;
        data1.windDir = weatherData.windDir;
        data1.tempSHT = weatherData.tempSHT;
        data1.tempSoil = weatherData.tempSoil;
        data1.humidity = weatherData.humidity;
        data1.pressure = weatherData.pressure;
        data1.light = weatherData.light;
        data1.windSpeed = weatherData.windSpeed;
        data2.rain = weatherData.rain;
        data2.battery = weatherData.battery;
        data2.epoch = weatherData.epoch;
        data2.solarJ = weatherData.solarJ;
        data2.solarW = weatherData.solarW;
        while (dataSent < 2 && sendTimeout > 0)
        {
          if (communication_Transmit(dataStructList[dataSent], dataStructListSize[dataSent], rxBuffer))
          {
            if (rxBuffer[0] == SYNC_HEADER)
            {
              dataSent++;
              memcpy(&syncStruct, rxBuffer, sizeof(syncStruct));
            }
          }
          char temp[10];
          sprintf(temp, "SEND %d", sendTimeout--);
          glassLCD_WriteData(temp);
          glassLCD_Update();
          HAL_Delay(1000);
        }
        RF24_flush_rx();
        RF24_flush_tx();
        RF24_powerDown();
      }
      if (dataSent == 0)
      {
        RTC_SetAlarmEpoch(RTC_GetTime() + sendInterval - 25,
            RTC_ALARMMASK_DATEWEEKDAY);
      }
      else
      {
        RTC_SetTime(syncStruct.myEpoch);
        RTC_SetAlarmEpoch(syncStruct.sendEpoch, RTC_ALARMMASK_DATEWEEKDAY);
        sendInterval = syncStruct.sendEpoch - syncStruct.myEpoch;
      }
    }

    if (interruptButton & GPIO_PIN_8)
    {
      interruptButton &= ~(GPIO_PIN_8);
      k++;
      k = k % 7;
    }

    if (interruptButton & GPIO_PIN_1)
    {
      interruptButton &= ~(GPIO_PIN_1);
      readWeatherData(&currentWeatherData, measurementTable[k]);
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Configure LSE Drive Capability 
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc.Instance = ADC1;
  hadc.Init.OversamplingMode = ENABLE;
  hadc.Init.Oversample.Ratio = ADC_OVERSAMPLING_RATIO_16;
  hadc.Init.Oversample.RightBitShift = ADC_RIGHTBITSHIFT_4;
  hadc.Init.Oversample.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV64;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.SamplingTime = ADC_SAMPLETIME_12CYCLES_5;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerFrequencyMode = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted. 
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted. 
  */
  sConfig.Channel = ADC_CHANNEL_1;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted. 
  */
  sConfig.Channel = ADC_CHANNEL_4;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00000E14;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable the Alarm A 
  */
  sAlarm.AlarmTime.Hours = 0;
  sAlarm.AlarmTime.Minutes = 0;
  sAlarm.AlarmTime.Seconds = 0;
  sAlarm.AlarmTime.SubSeconds = 0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 60;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 65535;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, EN_3V3SW_Pin|NRF24_CE_Pin|NRF24_CSN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BAT_M_EN_GPIO_Port, BAT_M_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : EN_3V3SW_Pin */
  GPIO_InitStruct.Pin = EN_3V3SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(EN_3V3SW_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BAT_M_EN_Pin */
  GPIO_InitStruct.Pin = BAT_M_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BAT_M_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : WS_DIN_Pin */
  GPIO_InitStruct.Pin = WS_DIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(WS_DIN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : NRF24_CE_Pin NRF24_CSN_Pin */
  GPIO_InitStruct.Pin = NRF24_CE_Pin|NRF24_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SI1147_INT_Pin */
  GPIO_InitStruct.Pin = SI1147_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SI1147_INT_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 4 */
void writeError(uint8_t _e, uint8_t _forceSleep)
{
  char _c[9];
  sprintf(_c, errStr, _e);
  glassLCD_WriteData(_c);
  glassLCD_Update();
  HAL_Delay(50);
  if (_forceSleep) HAL_PWR_EnterSTANDBYMode();
}

void readWeatherData(struct measruementHandle *_w, uint16_t _flags)
{
  if (_flags & TEMP_MEASUREMENT) _w->tempSHT = SHT21_ReadTemperature();
  if (_flags & HUMIDITY_MEASUREMENT)
  {
	  _w->humidity = SHT21_ReadHumidity();
	  if (_w->humidity > 100) _w->humidity = 100;
  }
  if (_flags & PRESSURE_MEASUREMENT)
  {
    _w->pressure = 0;
    for (int i = 0; i < 4; i++)
    {
      _w->pressure += BMP180_ReadPressure();
    }
    _w->pressure /= 4;
  }
  if (_flags & SOLAR_MEASUREMENT) getSolarData(&(_w->solarJ), &(_w->solarW));
  if (_flags & UV_MEASUREMENT)
  {
    Si1147_ForceUV();
    _w->uv = Si1147_GetUV();
  }
  if (_flags & UV_MEASUREMENT)
  {
    // Force the measurement of UV but it also measures visible light
    Si1147_ForceUV();
    _w->light = Si1147_GetVis() * 0.282 * 16.5;
  }
  if (_flags & WINDSPEED_MEASUREMENT) _w->windSpeed = getWindSpeed();
  if (_flags & WINDDIR_MEASUREMENT) _w->windDir = getWindDir(ADC_CHANNEL_1, windDirCalibration);
  if (_flags & BATTERY_MEASUREMENT) _w->battery = getBatteryVoltage();
  _w->epoch = (uint32_t)RTC_GetTime();
}

// Read solar data in Joules/cm2 and W/m2
void getSolarData(double *_energyJ, double *_energy)
{
  // Turn on supply to the OpAmp
  HAL_GPIO_WritePin(EN_3V3SW_GPIO_Port, EN_3V3SW_Pin, GPIO_PIN_SET);
  HAL_Delay(10);

  uint32_t rawAdc;
  double voltage;
  double current;

  // Get the measurement for  the solar cell
  rawAdc = getADC(ADC_CHANNEL_0);

  // Turn off supply to the OpAmp
  HAL_GPIO_WritePin(EN_3V3SW_GPIO_Port, EN_3V3SW_Pin, GPIO_PIN_RESET);

  // Calculations for voltage, current and energy
  voltage = (rawAdc / 4095.0 * 3.3) - 0.004;

  if (voltage < 0) voltage = 0;

  current = voltage / (220 / 4.7);
  *_energy = current / 40E-3 * 1000;
  *_energyJ = (1 / 1E4) * (*_energy) * 600;
}

uint32_t getADC(uint32_t _ch)
{
  uint32_t _result;
  ADC_ChannelConfTypeDef ch = {0};

  // Calibrate ADC
  HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED);

  // Select the channel (in order to work properly, on all channels ranks have to be set on RANK_NONE!)
  ch.Channel = _ch;
  ch.Rank = ADC_RANK_CHANNEL_NUMBER;
  HAL_ADC_ConfigChannel(&hadc, &ch);

  // Start ADC conversion of selected channel
  HAL_ADC_Start(&hadc);

  // Wait for conversion to be complete
  HAL_ADC_PollForConversion(&hadc, 1000);

  // Get the RAW ADC value
  _result = HAL_ADC_GetValue(&hadc);

  // Stop the ADC
  HAL_ADC_Stop(&hadc);

  // Remove channel from rank
  ch.Rank = ADC_RANK_NONE;
  HAL_ADC_ConfigChannel(&hadc, &ch);

  // Return the result
  return _result;
}

float getWindSpeed()
{
  // Coefficients for linear regression ((https://keisan.casio.com/exec/system/14059929550941))
  // Calculation for frequency to wind speed  y = A + (B * x) (x = Frequency, y = wind speed in m/s)
  double Acoef = 0;
  double Bcoef = 0.31413320680643629774139228;

  // Turn on supply to the wind speed sensor
  HAL_GPIO_WritePin(EN_3V3SW_GPIO_Port, EN_3V3SW_Pin, GPIO_PIN_SET);
  HAL_Delay(5);

  // Activate the timer
  HAL_TIM_Base_Start(&htim6);

  // Get inital state of the pin
  uint8_t _initState = HAL_GPIO_ReadPin(WS_DIN_GPIO_Port, WS_DIN_Pin);

  // Get the current sysTick time (needed for timeout)
  uint32_t _timeout = HAL_GetTick();
  uint32_t _period = 0;

  // Wait for the edge of the signal (doesn't matter if is rising or falling)
  while ((HAL_GPIO_ReadPin(WS_DIN_GPIO_Port, WS_DIN_Pin) == _initState) && ((HAL_GetTick() - _timeout) < 1000));
  if ((HAL_GetTick() - _timeout) >= 1000) return 0;

  // Reset the timer counter and measure the time until next edge of the signal
  htim6.Instance->CNT = 0;
  while ((HAL_GPIO_ReadPin(WS_DIN_GPIO_Port, WS_DIN_Pin) != _initState) && (htim6.Instance->CNT < 65530));
  _period += (htim6.Instance->CNT);

  // Reset the timer counter and measure the time until next edge of the signal
  htim6.Instance->CNT = 0;
  while ((HAL_GPIO_ReadPin(WS_DIN_GPIO_Port, WS_DIN_Pin) == _initState) && (htim6.Instance->CNT < 65530));
  _period += (htim6.Instance->CNT);

  // Stop the timer
  HAL_TIM_Base_Stop(&htim6);

  // Turn on supply to the wind speed sensor
  HAL_GPIO_WritePin(EN_3V3SW_GPIO_Port, EN_3V3SW_Pin, GPIO_PIN_RESET);

  // Calculate the frequency in hertz and return the result. 0.988533017 is  calibration value (tested with function generator)
  double freqHz = ((double)(1 / (_period * 1E-6 * 15))) * 0.988533017;

  // Calculate and return wind speed in m/s
  return (Acoef + (freqHz * Bcoef));
}

int16_t getWindDir(uint32_t _pin, int16_t _offset)
{
  // Turn on the supply to the wind direction sensor (AS5600)
  HAL_GPIO_WritePin(EN_3V3SW_GPIO_Port, EN_3V3SW_Pin, GPIO_PIN_SET);
  HAL_Delay(10);

  int16_t _voltage = getADC(_pin);
  int16_t _angle;
  _voltage = 1 / 4095.0 * _voltage * 3300;
  _angle = (int16_t) ((360.0 / 3300.0) * _voltage);
  _angle = _angle - _offset;
  if (_angle < 0)
    _angle = 360 + _angle;

  // Turn off the supply to the wind direction sensor (AS5600)
  HAL_GPIO_WritePin(EN_3V3SW_GPIO_Port, EN_3V3SW_Pin, GPIO_PIN_RESET);

  return _angle;
}

float getBatteryVoltage()
{
  float _result;
  HAL_GPIO_WritePin(BAT_M_EN_GPIO_Port, BAT_M_EN_Pin, GPIO_PIN_SET);
  HAL_Delay(10);
  _result = getADC(ADC_CHANNEL_4) * 1 / 4095.0 * 3.3 * 2;
  HAL_GPIO_WritePin(BAT_M_EN_GPIO_Port, BAT_M_EN_Pin, GPIO_PIN_RESET);
  return _result;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  interruptButton |= GPIO_Pin;
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  alarmInterruptFlag = 1;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
   tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
