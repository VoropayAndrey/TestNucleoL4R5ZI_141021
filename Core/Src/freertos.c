/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "HCITRANS.h"            /* HCI Transport Prototypes/Constants.       */
#include "HAL.h"
#include "usart.h"
#include "serialOutput.h"
#include "serialInput.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_COMMAND_LENGTH                         (64)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define LED_TOGGLE_RATE_SUCCESS                    (500) /* The LED Toggle    */
                                                         /* rate when the demo*/
                                                         /* successfully      */
                                                         /* starts up.        */
#define  COMMAND_LINE_ENTER_PRESSED               (-128) /* The return value  */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static unsigned int InputIndex;
static char         Input[MAX_COMMAND_LENGTH];

osThreadId_t btAudioTaskHandle;
const osThreadAttr_t btAudioTask_attributes = {
  .name = "btAudioTask",
  .stack_size = 4024 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t serialOutputTaskHandle;
const osThreadAttr_t serialOutputTask_attributes = {
  .name = "serialOutputTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static int GetInput(void);
   /* Sleep Mode Callback.                                          */
static void Sleep_Indication_Callback(Boolean_t SleepAllowed, unsigned long CallbackParameter);
   /* Application Tasks.                                                */
static int DisplayCallback(int Length, char *Message);
static void ProcessCharacters(void *UserParameter);
static void StartBluetoothHSPTask(void *UserParameter);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartSerialOutputTask(void *argument);
extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  btAudioTaskHandle = osThreadNew(StartBluetoothHSPTask, NULL, &btAudioTask_attributes);
  serialOutputTaskHandle = osThreadNew(StartSerialOutputTask, NULL, &serialOutputTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  //MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
      //serialOutputPrint("Hello\n", 6);

	  HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
	  osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

void StartSerialOutputTask(void *argument) {
	HAL_UART_StateTypeDef state = 0;
	for(;;)
	{
		state = HAL_UART_GetState(&hlpuart1);
		if(state == HAL_UART_STATE_BUSY_TX) {

		} else {
			serialOutputProcess();
		}
		osDelay(25);
	}
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */



static int GetInput(void)
{
   char Char;
   int  Done;

   /* Initialize the Flag indicating a complete line has been parsed.   */
   Done = 0;

   /* Attempt to read data from the Console.                            */
   while((!Done) && (HAL_ConsoleRead(1, &Char)))
   {
      switch(Char)
      {
         case '\r':
         case '\n':
		 	/* The user just pressed 'Enter'. Return and print the prompt */
             if(!InputIndex)
             {
                 /* Set the return value to (-128) */
                 Done = COMMAND_LINE_ENTER_PRESSED;
                 break;
             }
            /* This character is a new-line or a line-feed character    */
            /* NULL terminate the Input Buffer.                         */
            Input[InputIndex] = '\0';
			Input[InputIndex+1] = '\0';

            /* Set Done to the number of bytes that are to be returned. */
            /* ** NOTE ** In the current implementation any data after a*/
            /*            new-line or line-feed character will be lost. */
            /*            This is fine for how this function is used is */
            /*            no more data will be entered after a new-line */
            /*            or line-feed.                                 */
            Done       = (InputIndex-1);
            InputIndex = 0;
            break;
         case 0x08:
            /* Backspace has been pressed, so now decrement the number  */
            /* of bytes in the buffer (if there are bytes in the        */
            /* buffer).                                                 */
            if(InputIndex)
            {
               InputIndex--;
               HAL_ConsoleWrite(3, "\b \b");
            }
            break;
		 case 0x7F:
            /* Backspace has been pressed, so now decrement the number  */
            /* of bytes in the buffer (if there are bytes in the        */
            /* buffer).                                                 */
            if(InputIndex)
            {
               InputIndex--;
               HAL_ConsoleWrite(3, "\b \b");
            }
            break;
         default:
            /* Accept any other printable characters.                   */
            if((Char >= ' ') && (Char <= '~'))
            {
               /* Add the Data Byte to the Input Buffer, and make sure  */
               /* that we do not overwrite the Input Buffer.            */
               Input[InputIndex++] = Char;
               HAL_ConsoleWrite(1, &Char);

               /* Check to see if we have reached the end of the buffer.*/
               if(InputIndex == (MAX_COMMAND_LENGTH-1))
               {
                  Input[InputIndex] = 0;
                  Done              = (InputIndex-1);
                  InputIndex        = 0;
               }
            }
            break;
      }
   }

   return(Done);
}

static void Sleep_Indication_Callback(Boolean_t SleepAllowed, unsigned long CallbackParameter)
{
   /* Verify parameters. */
   if(CallbackParameter)
   {
      if(SleepAllowed)
      {
         /* Attempt to suspend the transport. */
         HCITR_COMSuspend(1);
      }
   }
}

static int DisplayCallback(int Length, char *Message)
{
   while(!HAL_ConsoleWrite(Length, Message))
   {
	   BTPS_Delay(1);
   }

    return TRUE;
}

/* The following function processes terminal input.					*/
static void ProcessCharacters(void *UserParameter)
{
  /* Check to see if we have a command to process.					   */
  int ret_val = GetInput();
  if(ret_val > 0)
  {
	 /* Attempt to process a character. 							   */
	 ProcessCommandLine(Input);
  }
  else if ((COMMAND_LINE_ENTER_PRESSED) == ret_val)
  {
	  Display(("\r\nA3DP+SNK>"));
  }
}

void StartBluetoothHSPTask(void *UserParameter) {
   int                           Result;
   BTPS_Initialization_t         BTPS_Initialization;
   HCI_DriverInformation_t       HCI_DriverInformation;
   HCI_HCILLConfiguration_t      HCILLConfig;
   HCI_Driver_Reconfigure_Data_t DriverReconfigureData;

   /* Configure the UART Parameters.                                    */
   HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, 2, 115200, cpHCILL_RTS_CTS);
   HCI_DriverInformation.DriverInformation.COMMDriverInformation.InitializationDelay = 100;

   /* Set up the application callbacks.                                 */
   BTPS_Initialization.MessageOutputCallback = DisplayCallback;

   /* Initialize the application.                                       */
   if((Result = InitializeApplication(&HCI_DriverInformation, &BTPS_Initialization)) > 0)
   {
	  /* Register a sleep mode callback if we are using HCILL Mode.     */
	  if((HCI_DriverInformation.DriverInformation.COMMDriverInformation.Protocol == cpHCILL) || (HCI_DriverInformation.DriverInformation.COMMDriverInformation.Protocol == cpHCILL_RTS_CTS))
	  {
		 HCILLConfig.SleepCallbackFunction        = Sleep_Indication_Callback;
		 HCILLConfig.SleepCallbackParameter       = 0;
		 DriverReconfigureData.ReconfigureCommand = HCI_COMM_DRIVER_RECONFIGURE_DATA_COMMAND_CHANGE_HCILL_PARAMETERS;
		 DriverReconfigureData.ReconfigureData    = (void *)&HCILLConfig;

		 /* Register the sleep mode callback.  Note that if this        */
		 /* function returns greater than 0 then sleep is currently     */
		 /* enabled.                                                    */
		 Result = HCI_Reconfigure_Driver((unsigned int)Result, FALSE, &DriverReconfigureData);
		 if(Result > 0)
		 {
			/* Flag that sleep mode is enabled.                         */
			Display(("Sleep is allowed.\r\n"));
		 }
	  }

			//BTPS_CreateThread(ToggleLED, 116, NULL);

	  /* Loop forever and process UART characters.                      */
	  while(1)
	  {
		  //CommandLineInterpreter("Pait\n\r");
		 ProcessCharacters(NULL);
		 //Display(("\r\nHello"));
		 BTPS_Delay(20);
	  }
   }
   while(1) {
	   osDelay(10);
   }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
