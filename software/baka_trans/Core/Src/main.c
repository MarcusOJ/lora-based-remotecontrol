/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "log.h"
#include "usbd_cdc_if.h"
#include "string.h"
#include "sx1262.h"
#include "messenger.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef void (*MessageHandler)(const char*);
void masterCommandHandler(uint16_t bytesAvailable);

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */


#define USB_CDC_BUFFER_LEN 0xFF
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
typedef struct {
    char* command;
    MessageHandler handler;
} CommandEntry;

// Define a table of command entries
CommandEntry commandTable[] = {
		// Debug commands
    {"status", sx1262_get_status_log},
	{"standby", sx1262_standby_rc},
	{"receive", sx1262_receive},
	{"raw-transmit", sx1262_transmit},
	{"transmit", radio_transmit},
	{"buffer-read", sx1262_buffer_read},
	{"buffer-status", sx1262_buffer_status},
	{"packet-stats", sx1262_packet_stats},
	{"lora-packet-status", sx1262_lora_packet_status},
	{"irq", sx1262_get_irq_status},
	{"errors", sx1262_get_device_errors},
	{"cad", sx1262_channel_actitity_detection},
	{"clear-irq", sx1262_clear_irq_all},
    // Transceiver commands
	{"transmit-get-data", get_site_data},
	{"transmit-get-battery-state", get_site_battery_state},
	{"transmit-ping", ping},
	{"transmit-sync-rtc", sync_rtc}
};


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
  MX_SPI2_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */



  uint16_t bytesAvailable;
  startup_sequence();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  if ((bytesAvailable = CDC_GetRxBufferBytesAvailable_FS()) > 0) {
		  masterCommandHandler(bytesAvailable);
	  }
	  if(rxDone){
		  rxDone = 0;
		  sx1262_clear_irq(SX126X_IRQ_RX_DONE);
		  LOG_TRACE("RX Done");

		  radio_message_processor();
	  }
	  if(txDone){
		  txDone = 0;
		  sx1262_clear_irq(SX126X_IRQ_TX_DONE);
		  LOG_TRACE("TX Done");
	  }

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void masterCommandHandler(uint16_t bytesAvailable){

	// USB receive buffer
	char rxData[USB_CDC_BUFFER_LEN];
	memset(rxData, 0, USB_CDC_BUFFER_LEN);

	// USB command and argument buffers
	char command[sizeof(rxData)];
	char argument[sizeof(rxData)];

	uint16_t bytesToRead = bytesAvailable >= USB_CDC_BUFFER_LEN ? USB_CDC_BUFFER_LEN : bytesAvailable;

	if (CDC_ReadRxBuffer_FS(rxData, bytesToRead) == USB_CDC_RX_BUFFER_OK) {
		//while (CDC_Transmit_FS(rxData, bytesToRead) == USBD_BUSY);
		rxData[bytesToRead] = '\0';
		char value[sizeof(rxData)];
		for (size_t i = 0; i < sizeof(rxData); i++) {
			value[i] = (char)rxData[i];
		}

		separate_string(rxData, command, argument);
		LOG_TRACE("command: %s - argument: %s", command, argument);

		executeCommand(command, argument);
	}
}

void executeCommand(const char* command, const char* argument) {
    // Iterate through the command table
    for (size_t i = 0; i < sizeof(commandTable) / sizeof(commandTable[0]); i++) {
        // Check if the command matches any entry in the command table
        if (strcmp(command, commandTable[i].command) == 0) {
            // Call the corresponding handler function
            commandTable[i].handler(argument);
            return;
        }
    }
    // If no matching command is found
    LOG_TRACE("Unknown command: %s\n", command);
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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
