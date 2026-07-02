/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/** * @brief Estructura para el mapeo de hardware.
 * Permite desacoplar la lógica de control de los pines físicos.
 */
typedef struct{
	GPIO_TypeDef* port;  	// Puntero a la estructura de registros del puerto (GPIOA, GPIOB, etc.)
	uint16_t pin;       	// Máscara del pin (GPIO_PIN_0, etc.)
} GPIO_Config_t;


/** * @brief Definición de estados para la MEF.
 */
typedef enum{
	Estado_Encendido = 0,
	Estado_Apagado
} Estado_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/** * @brief Arreglo de configuración de hardware.
 * Se define como 'const' para residir en Flash y ahorrar RAM.
 */
const GPIO_Config_t leds_array [] = {
		{LD1_GPIO_Port, LD1_Pin},
		{LD2_GPIO_Port, LD2_Pin},
		{LD3_GPIO_Port, LD3_Pin}
};

const uint8_t cant_leds = sizeof (leds_array) / sizeof(leds_array[0]);	// Se calcula automaticamente la cantidad de elementos del array
const uint32_t tiempo = 200;											// Valor constante para las transciciones de Encendido-Apagado
static int indice = 0;													// Variable que iterara el arreglo
static Estado_t estado = Estado_Encendido;								// Estado inicial de la MEF
static int8_t sentido = 1;												// Variable de Estado que determina el sentido de encendido de los leds
static bool btn_presionado = false;										// Bandera para la deteccion de flancos

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

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
	MX_USART3_UART_Init();
	/* USER CODE BEGIN 2 */

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

				/* 1. Detección de flanco (no bloqueante) */
		        /**
		         * @brief Detector de flanco positivo para el pulsador de usuario.
		         * @details Implementa una lógica de captura de evento única por pulsación (edge).
		         * Al usar una bandera, se evita la repetición de la acción mientras el usuario
		         * mantiene el botón presionado, garantizando una respuesta determinista.
		         */
		        if (HAL_GPIO_ReadPin(USER_BTN_GPIO_Port, USER_BTN_Pin) == GPIO_PIN_SET) {
		            if (!btn_presionado) {
		                sentido *= -1;         // Inversión de dirección (sentido de giro)
		                btn_presionado = true; // Bloquea la acción hasta que se suelte el botón
		            }
		        } else {
		            btn_presionado = false;    // Resetea la bandera al soltar el botón
		        }

		        /* 2. Máquina de Estados (MEF) */
		        /**
		         * @brief Máquina de Estados Finitos para la secuencia de LEDs.
		         * @details Gestiona el ciclo de encendido/apagado y la actualización del índice.
		         * La lógica de navegación es bidireccional, dependiendo de la variable 'sentido'.
		         */
		        switch (estado) {
		        case Estado_Encendido:
		            HAL_GPIO_WritePin(leds_array[indice].port, leds_array[indice].pin, GPIO_PIN_SET);
		            HAL_Delay(tiempo);
		            estado = Estado_Apagado;	// Trancision obligatoria al siguiente estado (apagado)
		            break;

		        case Estado_Apagado:
		            HAL_GPIO_WritePin(leds_array[indice].port, leds_array[indice].pin, GPIO_PIN_RESET);
		            HAL_Delay(tiempo);

		            // Actualización del índice considerando la dirección actual
		            indice += sentido;

		            // Lógica circular para mantener el índice dentro de los límites del arreglo
		            if (indice >= cant_leds) {
		                indice = 0;
		            } else if (indice < 0) {
		                indice = cant_leds - 1;
		            }

		            estado = Estado_Encendido;	// Retorna al estado de Encendido
		            break;

		        default:
		            // Recuperación ante estados indeterminados: Apaga todo y reinicia secuencia
		            for (uint8_t j = 0; j < cant_leds; j++) {
		                HAL_GPIO_WritePin(leds_array[j].port, leds_array[j].pin, GPIO_PIN_RESET);
		            }
					estado = Estado_Encendido;	// Se reinicia al estado inicial
					indice = 0;					// Reinicia al primer elemento del array leds_array
		            break;
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

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 180;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void)
{

	/* USER CODE BEGIN USART3_Init 0 */

	/* USER CODE END USART3_Init 0 */

	/* USER CODE BEGIN USART3_Init 1 */

	/* USER CODE END USART3_Init 1 */
	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart3) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART3_Init 2 */

	/* USER CODE END USART3_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : USER_BTN_Pin */
	GPIO_InitStruct.Pin = USER_BTN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USER_BTN_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
	GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_PowerSwitchOn_Pin */
	GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_OverCurrent_Pin */
	GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
