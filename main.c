///*****************************************************************************
/// main.c
///-----------------------------------------------------------------------------
/// @brief Hardware specification for WeAct Black Pill V2.0 (STM32F411CEU6)
///
/// * Hardware Configurations & Memory Limits:
///   - Core: Arm Cortex-M4 @ 100MHz Max
///   - FLASH: 512 KiB | SRAM: 128 KiB
///   - Oscillators: HSE = 25 MHz | LSE = 32.768 kHz | HSI = 16 MHz
///
/// * On-Board Peripherals & Per-Pin Mapping:
///   - KEY Button:   PA0  (Active Low, External Pull-up needed/expected)
///   - User LED:     PC13 (Active Low / Sink mode)
///   - USB C D-/D+:  PA11 / PA12
///   - SWD Header:   PA13 (SWDIO) / PA14 (SWCLK)
///   - SPI Flash/EEPROM Footprint (U3):
///   - CS:   PA4
///   - CLK:  PA5
///   - MISO: PB4 (DO)
///   - MOSI: PA7 (DI)
///   - OSC_IN : PC14
///   - OSC_OUT : PC15
///
/// * @warning Power Collision Risk:
///   +5V pins are directly coupled to the USB VBUS without protection diodes.
///   DO NOT power the board via USB and external 5V simultaneously.
///
/// * Semihosting
///   - openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
///   - arm-none-eabi-gdb -ex "target extended-remote localhost:3333" -ex "monitor arm semihosting enable" -ex "continue"
///
///   WARN: system is halted as printf causes breakpoint. So gdb is required to resume.
///
///*****************************************************************************

#include <stdint.h>
#include <stdio.h>
#include "main.h"

void clock_init();
// Semihosting
extern void initialise_monitor_handles(void);

void main(void)
{
  HAL_Init();
  clock_init();
  SystemCoreClockUpdate(); // Update the internal clock frequency variable
  // Init for semihosting
  initialise_monitor_handles();

  // Initialize LED GPIO
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitTypeDef gpio_init = {0};
  gpio_init.Pin = LED_PIN;
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_LOW;
  gpio_init.Alternate = 0;

  HAL_GPIO_Init(LED_PORT, &gpio_init);

  while(1)
  {
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);

    // causes breakpoint
    printf("[%.3f] Hello, World!\r\n", HAL_GetTick()/1000.0f);

    HAL_Delay(500);
  }
}

void clock_init()
{
  /* By default HSI (16 MHz RC oscillator) is selected as system clock.
   * We want to use the HSE (25 MHz external crystal connected to OSC_IN/OSC_OUT)
   * through the PLL to get 100 MHz system clock.
   */

  // Enable power controller and set voltage scale mode 1
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  // Configure PLL dividers and multiplier
  /* Input to PLL should be 1-2 MHz (preferably 1 MHz). Choosing M=25 gives
   * us 25 MHz / 25 = 1 MHz.
   * The output of the PLL should be 100-438 MHz, so setting the feedback
   * multiplier to N=200 gives us 1 MHz * 200 = 200 MHz.
   * The system clock should be 100 MHz. Choosing P=2 gives us
   * 200 MHz / 2 = 100 MHz
   */
  RCC_OscInitTypeDef osc_init = {0};
  osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc_init.HSEState = RCC_HSE_ON;
  osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  osc_init.PLL.PLLState = RCC_PLL_ON;
  osc_init.PLL.PLLM = 25;
  osc_init.PLL.PLLN = 200;
  osc_init.PLL.PLLP = RCC_PLLP_DIV2;
  osc_init.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
  {
    while(1);
  }

  /* Set PLL output as the source for the system clock.
   * Since APB1 clock must not be more than 50 MHz, set the PCKL1 divider to 2.
   */
  RCC_ClkInitTypeDef clock_init = {0};
  clock_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_HCLK;
  clock_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clock_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clock_init.APB1CLKDivider = RCC_HCLK_DIV2;
  clock_init.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&clock_init, FLASH_LATENCY_3) != HAL_OK) // Configure flash controller for 3V3 supply and 100 MHz -> 3 wait states
  {
    while(1);
  }
}
