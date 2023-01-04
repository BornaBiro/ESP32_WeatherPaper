#include "sleep.h"

void Sleep_LightSleep()
{
	HAL_PWR_DisablePVD();
	HAL_PWREx_EnableUltraLowPower();

	// Disable HAL_Tick (it triggers every 1ms, even from sleep)
	HAL_SuspendTick();

	// Disable all unused periph. in sleep mode
	HAL_I2C_DeInit(&hi2c1);
	HAL_SPI_DeInit(&hspi1);
	HAL_UART_DeInit(&huart2);
	HAL_ADC_DeInit(&hadc);

	// Enter "light sleep" mode and wait for Interrupt to wake up (WFI - Wait For Interrupt)
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

	// Recover from "light sleep" mode

	// First set up all clock once again
	SystemClock_Config();

	// Re-activate HAL_Tick
	HAL_ResumeTick();

	// Re-Init all prev. disabled periph.
	HAL_I2C_Init(&hi2c1);
	HAL_SPI_Init(&hspi1);
	HAL_UART_Init(&huart2);
	HAL_ADC_Init(&hadc);
}
