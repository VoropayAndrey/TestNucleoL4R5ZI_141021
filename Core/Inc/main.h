/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SS1BTPS.h"             /* Main SS1 Bluetooth Stack Header.          */
#include "SS1BTVS.h"             /* Vendor Specific Prototypes/Constants.     */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define Display(_x)                 do { BTPS_OutputMessage _x; } while(0)

/* Error Return Codes.                                               */

/* Error Codes that are smaller than these (less than -1000) are     */
/* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define APPLICATION_ERROR_INVALID_PARAMETERS             (-1000)
#define APPLICATION_ERROR_UNABLE_TO_OPEN_STACK           (-1001)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define HCI2_RTS_Pin GPIO_PIN_1
#define HCI2_RTS_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD
#define HCI2_CTS_Pin GPIO_PIN_11
#define HCI2_CTS_GPIO_Port GPIOD
#define HCI2_CTS_EXTI_IRQn EXTI15_10_IRQn
#define STLINK_TX_Pin GPIO_PIN_7
#define STLINK_TX_GPIO_Port GPIOG
#define STLINK_RX_Pin GPIO_PIN_8
#define STLINK_RX_GPIO_Port GPIOG
#define USB_SOF_Pin GPIO_PIN_8
#define USB_SOF_GPIO_Port GPIOA
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define USB_ID_Pin GPIO_PIN_10
#define USB_ID_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SDIO_PLG_Pin GPIO_PIN_0
#define SDIO_PLG_GPIO_Port GPIOD
#define BT_HCI_RESET_Pin GPIO_PIN_1
#define BT_HCI_RESET_GPIO_Port GPIOD
#define HCI1_CTS_Pin GPIO_PIN_3
#define HCI1_CTS_GPIO_Port GPIOD
#define HCI1_CTS_EXTI_IRQn EXTI3_IRQn
#define HCI1_RTS_Pin GPIO_PIN_4
#define HCI1_RTS_GPIO_Port GPIOD
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
/* The following enumerated type is used with the                    */
/* SendRemoteControlCommand() function to specify the type of remote */
/* control command to send to a connected Bluetooth device.          */
typedef enum
{
	rcPlay,
	rcPause,
	rcPlayPause,
	rcNext,
	rcBack,
	rcVolumeUp,
	rcVolumeDown
} RemoteControlCommand_t;

/* Bluetooth Protocol Demo Application Major Version Release Number.  */
#define DEMO_APPLICATION_MAJOR_VERSION_NUMBER      0

 /* Bluetooth Protocol Demo Application Minor Version Release Number.  */
#define DEMO_APPLICATION_MINOR_VERSION_NUMBER      1

   /* Constants used to convert numeric constants to string constants   */
   /* (used in MACRO's below).                                          */
#define VERSION_NUMBER_TO_STRING(_x)               #_x
#define VERSION_CONSTANT_TO_STRING(_y)             VERSION_NUMBER_TO_STRING(_y)

   /*  Demo Application Version String of the form "a.b"                */
   /* where:                                                            */
   /*    a - DEMO_APPLICATION_MAJOR_VERSION_NUMBER                      */
   /*    b - DEMO_APPLICATION_MINOR_VERSION_NUMBER                      */

#define DEMO_APPLICATION_VERSION_STRING            VERSION_CONSTANT_TO_STRING(DEMO_APPLICATION_MAJOR_VERSION_NUMBER) "." VERSION_CONSTANT_TO_STRING(DEMO_APPLICATION_MINOR_VERSION_NUMBER)


   /* The following function is used to initialize the application      */
   /* instance.  This function should open the stack and prepare to     */
   /* execute commands based on user input.  The first parameter passed */
   /* to this function is the HCI Driver Information that will be used  */
   /* when opening the stack and the second parameter is used to pass   */
   /* parameters to BTPS_Init.  This function returns the               */
   /* BluetoothStackID returned from BSC_Initialize on success or a     */
   /* negative error code (of the form APPLICATION_ERROR_XXX).          */
int InitializeApplication(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization);

   /* The following function is used to process a command line string.  */
   /* This function takes as it's only parameter the command line string*/
   /* to be parsed and returns TRUE if a command was parsed and executed*/
   /* or FALSE otherwise.                                               */
Boolean_t ProcessCommandLine(char *String);

   /* The following function is used to send the specified remote       */
   /* control to the currently connected remote control device.         */
int SendRemoteControlCommand(RemoteControlCommand_t Command);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
