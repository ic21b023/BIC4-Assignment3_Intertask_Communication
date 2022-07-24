/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "serialprotocol.h"
#include "string.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

typedef struct {
	const char * name;     	/*!< Taskname */
	int value;				/*!< Value */
} queueitem_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define RxBuffer_UART2_SIZE 1	/*!< Groesse des Empfangspuffers */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 UART_HandleTypeDef huart2;

/* Definitions for producerTask1 */
osThreadId_t producerTask1Handle;
uint32_t producerTask1Buffer[ 256 ];
osStaticThreadDef_t producerTask1ControlBlock;
const osThreadAttr_t producerTask1_attributes = {
  .name = "producerTask1",
  .cb_mem = &producerTask1ControlBlock,
  .cb_size = sizeof(producerTask1ControlBlock),
  .stack_mem = &producerTask1Buffer[0],
  .stack_size = sizeof(producerTask1Buffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for producerTask2 */
osThreadId_t producerTask2Handle;
uint32_t producerTask2Buffer[ 256 ];
osStaticThreadDef_t producerTask2ControlBlock;
const osThreadAttr_t producerTask2_attributes = {
  .name = "producerTask2",
  .cb_mem = &producerTask2ControlBlock,
  .cb_size = sizeof(producerTask2ControlBlock),
  .stack_mem = &producerTask2Buffer[0],
  .stack_size = sizeof(producerTask2Buffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for producerTask3 */
osThreadId_t producerTask3Handle;
uint32_t producerTask3Buffer[ 256 ];
osStaticThreadDef_t producerTask3ControlBlock;
const osThreadAttr_t producerTask3_attributes = {
  .name = "producerTask3",
  .cb_mem = &producerTask3ControlBlock,
  .cb_size = sizeof(producerTask3ControlBlock),
  .stack_mem = &producerTask3Buffer[0],
  .stack_size = sizeof(producerTask3Buffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for monitoringTask */
osThreadId_t monitoringTaskHandle;
uint32_t monitoringTaskBuffer[ 256 ];
osStaticThreadDef_t monitoringTaskControlBlock;
const osThreadAttr_t monitoringTask_attributes = {
  .name = "monitoringTask",
  .cb_mem = &monitoringTaskControlBlock,
  .cb_size = sizeof(monitoringTaskControlBlock),
  .stack_mem = &monitoringTaskBuffer[0],
  .stack_size = sizeof(monitoringTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for isrTask */
osThreadId_t isrTaskHandle;
uint32_t isrTaskBuffer[ 256 ];
osStaticThreadDef_t isrTaskControlBlock;
const osThreadAttr_t isrTask_attributes = {
  .name = "isrTask",
  .cb_mem = &isrTaskControlBlock,
  .cb_size = sizeof(isrTaskControlBlock),
  .stack_mem = &isrTaskBuffer[0],
  .stack_size = sizeof(isrTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for outputmessageQueue */
osMessageQueueId_t outputmessageQueueHandle;
uint8_t outputmessageQueueBuffer[ 10 * sizeof( queueitem_t ) ];
osStaticMessageQDef_t outputmessageQueueControlBlock;
const osMessageQueueAttr_t outputmessageQueue_attributes = {
  .name = "outputmessageQueue",
  .cb_mem = &outputmessageQueueControlBlock,
  .cb_size = sizeof(outputmessageQueueControlBlock),
  .mq_mem = &outputmessageQueueBuffer,
  .mq_size = sizeof(outputmessageQueueBuffer)
};
/* Definitions for intervalQueueT1 */
osMessageQueueId_t intervalQueueT1Handle;
uint8_t intervalQueueT1Buffer[ 10 * sizeof( uint32_t ) ];
osStaticMessageQDef_t intervalQueueT1ControlBlock;
const osMessageQueueAttr_t intervalQueueT1_attributes = {
  .name = "intervalQueueT1",
  .cb_mem = &intervalQueueT1ControlBlock,
  .cb_size = sizeof(intervalQueueT1ControlBlock),
  .mq_mem = &intervalQueueT1Buffer,
  .mq_size = sizeof(intervalQueueT1Buffer)
};
/* Definitions for intervalQueueT2 */
osMessageQueueId_t intervalQueueT2Handle;
uint8_t intervalQueueT2Buffer[ 16 * sizeof( uint32_t ) ];
osStaticMessageQDef_t intervalQueueT2ControlBlock;
const osMessageQueueAttr_t intervalQueueT2_attributes = {
  .name = "intervalQueueT2",
  .cb_mem = &intervalQueueT2ControlBlock,
  .cb_size = sizeof(intervalQueueT2ControlBlock),
  .mq_mem = &intervalQueueT2Buffer,
  .mq_size = sizeof(intervalQueueT2Buffer)
};
/* Definitions for intervalQueueT3 */
osMessageQueueId_t intervalQueueT3Handle;
uint8_t intervalQueueT3Buffer[ 16 * sizeof( uint32_t ) ];
osStaticMessageQDef_t intervalQueueT3ControlBlock;
const osMessageQueueAttr_t intervalQueueT3_attributes = {
  .name = "intervalQueueT3",
  .cb_mem = &intervalQueueT3ControlBlock,
  .cb_size = sizeof(intervalQueueT3ControlBlock),
  .mq_mem = &intervalQueueT3Buffer,
  .mq_size = sizeof(intervalQueueT3Buffer)
};
/* USER CODE BEGIN PV */
int value;
char RxBuffer_UART2[1]={0};
SERIALPROTOCOL_HandleTypeDef hserialprot;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void producerFunction1(void *argument);
void producerFunction2(void *argument);
void producerFunction3(void *argument);
void monitoringFunction(void *argument);
void isrFunction(void *argument);

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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, (uint8_t *)RxBuffer_UART2, 1);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of outputmessageQueue */
  outputmessageQueueHandle = osMessageQueueNew (10, sizeof(queueitem_t), &outputmessageQueue_attributes);

  /* creation of intervalQueueT1 */
  intervalQueueT1Handle = osMessageQueueNew (10, sizeof(uint32_t), &intervalQueueT1_attributes);

  /* creation of intervalQueueT2 */
  intervalQueueT2Handle = osMessageQueueNew (16, sizeof(uint32_t), &intervalQueueT2_attributes);

  /* creation of intervalQueueT3 */
  intervalQueueT3Handle = osMessageQueueNew (16, sizeof(uint32_t), &intervalQueueT3_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of producerTask1 */
  producerTask1Handle = osThreadNew(producerFunction1, NULL, &producerTask1_attributes);

  /* creation of producerTask2 */
  producerTask2Handle = osThreadNew(producerFunction2, NULL, &producerTask2_attributes);

  /* creation of producerTask3 */
  producerTask3Handle = osThreadNew(producerFunction3, NULL, &producerTask3_attributes);

  /* creation of monitoringTask */
  monitoringTaskHandle = osThreadNew(monitoringFunction, NULL, &monitoringTask_attributes);

  /* creation of isrTask */
  isrTaskHandle = osThreadNew(isrFunction, NULL, &isrTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_9;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
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

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/**
  * @brief UART_RxCpltCallback wird aufgerufen, wenn Rx-Buffer voll ist
  * @param UART_HandleTypeDef
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	/* isrTask forsetzen */
	BaseType_t xyieldRequired = xTaskResumeFromISR((TaskHandle_t)isrTaskHandle);
	portYIELD_FROM_ISR(xyieldRequired);
}

/**
  * @brief Callback für CMS-Commands (zum setzen der Intervalls der producerTasks)
  * @param SERIALPROTOCOL_TypeDef - Struct des seriellen Protokolls
  * @retval None
  */
uint8_t SERIALPROT_Command_CMS_Callback(SERIALPROTOCOL_HandleTypeDef *hserialprot)
{

	/* Prüfen ob das Kommando "cms" mit dem Parameter 1 "t1" eingegeben wurde um danach das Invervall nach Parameter 2 zu ändern */
	if(__SERIALPROT_IS_PARAMETER1(hserialprot,"t1")){

		uint32_t t1delay=atoi(hserialprot->Parameter2);

		/* Prüfen ob eine Zelle der Queue für den producerTask1 frei ist */
		if(osMessageQueueGetSpace(intervalQueueT1Handle)>0)
		{
			/* Wert des Intervalls für den producerTask1 in die Queue speichern */
			osMessageQueuePut(intervalQueueT1Handle, &t1delay, 0U, 10U);
		}
		return 0;

	/* Prüfen ob das Kommando "cms" mit dem Parameter 1 "t2" eingegeben wurde um danach das Invervall nach Parameter 2 zu ändern */
	}else if(__SERIALPROT_IS_PARAMETER1(hserialprot,"t2")){
		uint32_t t2delay=atoi(hserialprot->Parameter2);

		/* Prüfen ob eine Zelle der Queue für den producerTask2 frei ist */
		if(osMessageQueueGetSpace(intervalQueueT2Handle)>0)
		{
			/* Wert des Intervalls für den producerTask2 in die Queue speichern */
			osMessageQueuePut(intervalQueueT2Handle, &t2delay, 0U, 10UL);
		}
		return 0;

	/* Prüfen ob das Kommando "cms" mit dem Parameter 1 "t3" eingegeben wurde um danach das Invervall nach Parameter 2 zu ändern */
	}else if(__SERIALPROT_IS_PARAMETER1(hserialprot,"t3")){
		uint32_t t3delay=atoi(hserialprot->Parameter2);

		/* Prüfen ob eine Zelle der Queue für den producerTask3 frei ist */
		if(osMessageQueueGetSpace(intervalQueueT2Handle)>0)
		{
			/* Wert des Intervalls für den producerTask2 in die Queue speichern */
			osMessageQueuePut(intervalQueueT2Handle, &t3delay, 0U, 10U);
		}
		return 0;
	}
	return 1;
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_producerFunction1 */
/**
  * @brief  Function implementing the producerTask1 thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_producerFunction1 */
void producerFunction1(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	queueitem_t buffer;
	uint32_t delayT1=1500;

  for(;;)
  {

	  /* Prüfen ob eine Zelle in der outputmessageQueue frei ist. Wenn ja, den Wert der Integervariable erhöhen und
	   * diesen mit dem Tasknamen in die outputmessageQueue schreiben */
	  if(osMessageQueueGetSpace(outputmessageQueueHandle)>0)
	  {
		  value++;
		  buffer.name=producerTask1_attributes.name;
		  buffer.value=value;
		  osMessageQueuePut(outputmessageQueueHandle, &buffer, 0U, 10U);
	  }

	  /* Intervall aus der intervalQueueT1Handle auslesen */
	  uint32_t newdelay;
	  osStatus_t osStatus = osMessageQueueGet(intervalQueueT1Handle, &newdelay, NULL, 10U);

	  /* Wenn das Auslesen aus der intervalQueueT1Handle erfolgreiche war, dass Invervall von T1 aktualisieren */
	  if (osStatus == osOK)
	  {
		  delayT1=newdelay;
	  }

    osDelay(delayT1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_producerFunction2 */
/**
* @brief Function implementing the producerTask2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_producerFunction2 */
void producerFunction2(void *argument)
{
  /* USER CODE BEGIN producerFunction2 */
  /* Infinite loop */
	queueitem_t buffer;
	uint32_t delayT2=1500;

  for(;;)
  {
	  /* Prüfen ob eine Zelle in der outputmessageQueue frei ist. Wenn ja, den Wert der Integervariable erhöhen und
	   * diesen mit dem Tasknamen in die outputmessageQueue schreiben */
	  if(osMessageQueueGetSpace(outputmessageQueueHandle)>0)
	  {
		  value++;
		  buffer.name=producerTask2_attributes.name;
		  buffer.value=value;
		  osMessageQueuePut(outputmessageQueueHandle, &buffer, 0U, 10U);
	  }

	  /* Intervall aus der intervalQueueT2Handle auslesen */
	  uint32_t newdelay;
	  osStatus_t osStatus = osMessageQueueGet(intervalQueueT2Handle, &newdelay, NULL, 10U);

	  /* Wenn das Auslesen aus der intervalQueueT2Handle erfolgreiche war, dass Invervall von T2 aktualisieren */
	  if (osStatus == osOK)
	  {
		  delayT2=newdelay;
	  }

    osDelay(delayT2);
  }
  /* USER CODE END producerFunction2 */
}

/* USER CODE BEGIN Header_producerFunction3 */
/**
* @brief Function implementing the producerTask3 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_producerFunction3 */
void producerFunction3(void *argument)
{
  /* USER CODE BEGIN producerFunction3 */
  /* Infinite loop */
	queueitem_t buffer;
	uint32_t delayT3=1500;
  for(;;)
  {
	  /* Prüfen ob eine Zelle in der outputmessageQueue frei ist. Wenn ja, den Wert der Integervariable erhöhen und
	   * diesen mit dem Tasknamen in die outputmessageQueue schreiben */
	  if(osMessageQueueGetSpace(outputmessageQueueHandle)>0)
	  {
		  value++;
		  buffer.name=producerTask3_attributes.name;
		  buffer.value=value;
		  osMessageQueuePut(outputmessageQueueHandle, &buffer, 0U, 10U);
	  }

	  /* Intervall aus der intervalQueueT3Handle auslesen */
	  uint32_t newdelay;
	  osStatus_t osStatus = osMessageQueueGet(intervalQueueT3Handle, &newdelay, NULL, 10U);

	  /* Wenn das Auslesen aus der intervalQueueT3Handle erfolgreiche war, dass Invervall von T3 aktualisieren */
	  if (osStatus == osOK)
	  {
		  delayT3=newdelay;
	  }

    osDelay(delayT3);
  }
  /* USER CODE END producerFunction3 */
}

/* USER CODE BEGIN Header_monitoringFunction */
/**
* @brief Function implementing the monitoringTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_monitoringFunction */
void monitoringFunction(void *argument)
{
  /* USER CODE BEGIN monitoringFunction */
  /* Infinite loop */
	 queueitem_t buffer;
  for(;;)
  {

	  /* Nachrichten der producerTasks aus der outputmessageQueueHandle auslesen */
      osStatus_t result = osMessageQueueGet(outputmessageQueueHandle, &buffer, NULL, 10U);

      /* wenn der Auslesen aus der outputmessageQueueHandle erfolgreich war den Taskname und Wert ausgeben */
      if (result == osOK){

    	  HAL_UART_Transmit(&huart2, (uint8_t *)buffer.name, strlen(buffer.name),100);
    	  char * arrow = " -> ";
    	  HAL_UART_Transmit(&huart2,(uint8_t *)arrow, 4,100);
    	  char result[20];
    	  itoa(buffer.value,result,10);
    	  HAL_UART_Transmit(&huart2, (uint8_t *) result,  strlen(result),100);
    	  char * new_line = "\r\n";
    	  HAL_UART_Transmit(&huart2, (uint8_t *)new_line , 2,100);
      }

    osDelay(600);
  }
  /* USER CODE END monitoringFunction */
}

/* USER CODE BEGIN Header_isrFunction */
/**
* @brief Function implementing the isrTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_isrFunction */
void isrFunction(void *argument)
{
  /* USER CODE BEGIN isrFunction */
  /* Infinite loop */
  for(;;)
  {
	  /* isrTask anhalten */
	  vTaskSuspend(isrTaskHandle);

	  /* monitoringTask anhalten */
	  vTaskSuspend(monitoringTaskHandle);

	  /* Nachrichtenbuffer für die Konsolen-Nachricht */
	  	char exchangedMessage[55] ={0};

	  	/*
	  	 * MYLIB_SERIALPROT_XCHANGE -> Verarbeitet die eingegebenen Zeichen des UART und gibt das demenstspechende Ergebnis/Nachricht zurück
	  	 * hserialprot -> Objekt des Seriellen Protokolls
	  	 * RxBuffer -> Ein-Zeichen-Empfangspuffer
	  	 * exchangedMessage -> zurückgegebene Nachricht aufgrund der Eingaben von Rx bzw. auf der Konsole
	  	 * Die Verarbeitung erfolgt im Serialprotokoll-Modul (serialprotocol.c und serialprotocol.h)
	  	 */
	  	uint8_t result = SERIALPROTOCOL_Transmit(&hserialprot,RxBuffer_UART2,exchangedMessage);


	  	/* exchangedMessage an Putty/Konsole senden */
	  	if(HAL_UART_Transmit(&huart2, (uint8_t *) exchangedMessage,(uint16_t)strlen(exchangedMessage), 100)!= HAL_OK){Error_Handler();}

		if (result)
		{
			/* monitoringTask fortsetzen */
			vTaskResume(monitoringTaskHandle);
		}

	  	/* UART_Receive Interrupt aktivieren */
		if(HAL_UART_Receive_IT(&huart2,(uint8_t *) RxBuffer_UART2, RxBuffer_UART2_SIZE)!= HAL_OK){Error_Handler();}
  }
  /* USER CODE END isrFunction */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
