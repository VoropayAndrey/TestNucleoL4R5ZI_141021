/*****< main.c >***************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*      Copyright 2015 Texas Instruments Incorporated.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MAIN - Main application implementation.                                   */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/28/12  T. Cook        Initial creation.                               */
/*   08/03/15  D. Horowitz    Porting to STM32 from MSP430                    */
/******************************************************************************/

#include "FreeRTOS.h"            /* freeRTOS Kernal Prototypes/Constants.    */
#include "task.h"                /* freeRTOS Task Header.                    */


#include "Main.h"                 /* Main application header.                 */
#include "HAL.h"                  /* Function for Hardware Abstraction.       */
#include "HALCFG.h"               /* HAL Configuration Constants.             */
#include "HCITRANS.h"


#define MAX_COMMAND_LENGTH                         (64)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define  COMMAND_LINE_ENTER_PRESSED               (-128) /* The return value  */
                                                         /* for Enter char in */
                                                         /* GetInput function */


   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int InputIndex = 0;
static char         Input[MAX_COMMAND_LENGTH];

   /* Internal function prototypes.                                     */
static void *ToggleLED(void *UserParameter);
static int GetInput(void);

   /* Sleep Mode Callback.                                          */
static void Sleep_Indication_Callback(Boolean_t SleepAllowed, unsigned long CallbackParameter);

   /* Application Tasks.                                                */
static int DisplayCallback(int Length, char *Message);
static void ProcessCharacters(void *UserParameter);
static void MainThread(void *UserParameter);


   /* The following function is responsible for retrieving the Commands */
   /* from the Serial Input routines and copying this Command into the  */
   /* specified Buffer.  This function blocks until a Command (defined  */
   /* to be a NULL terminated ASCII string).  The Serial Data Callback  */
   /* is responsible for building the Command and dispatching the Signal*/
   /* that this function waits for.                                     */
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
            /* NULL terminate the Input Buffer and erase the following  */
			/* Char without the need of clearing the entire buffer      */
			/* before reading.                                          */
            Input[InputIndex]   = '\0';
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

               while(!HAL_ConsoleWrite(1, "\b"))
                  ;
               while(!HAL_ConsoleWrite(1, " "))
                  ;
               while(!HAL_ConsoleWrite(1, "\b"))
                  ;
            }
            break;
         case 0x7F:
            /* Backspace has been pressed, so now decrement the number  */
            /* of bytes in the buffer (if there are bytes in the        */
            /* buffer).                                                 */
            if(InputIndex)
            {
               InputIndex--;
               while(!HAL_ConsoleWrite(1, "\b")){}
               while(!HAL_ConsoleWrite(1, " ")){}
               while(!HAL_ConsoleWrite(1, "\b")){}
            }
            break;
         default:
            /* Accept any other printable characters.                   */
            if((Char >= ' ') && (Char <= '~'))
            {
               if(InputIndex >= (MAX_COMMAND_LENGTH-1))
               {
                  Input[InputIndex] = '\0';
                  Done              = (InputIndex-1);
                  InputIndex        = 0;
               }
               /* Add the Data Byte to the Input Buffer, and make sure  */
               /* that we do not overwrite the Input Buffer.            */
               Input[InputIndex++] = Char;
               while(!HAL_ConsoleWrite(1, &Char))
                  ;

            }
            break;
      }
   }

   return(Done);
}

    
   /* The following function is registered with the application so that */
   /* it can display strings to the debug UART.                         */
static int DisplayCallback(int Length, char *Message)
{
   while(!HAL_ConsoleWrite(Length, Message))
   {
	   BTPS_Delay(1);
   }

    return TRUE;
}




   
   /* The following function is registered with the application so that */
   /* it can get the current System Tick Count.                         */
   /* The following function processes terminal input.                  */
static void ProcessCharacters(void *UserParameter)
{
   /* Check to see if we have a command to process.                     */
   int ret_val = GetInput();
   if(ret_val > 0)
   {
      /* Attempt to process a character.                                */
      ProcessCommandLine(Input);
   }
   else if ((COMMAND_LINE_ENTER_PRESSED) == ret_val)
   {
       Display(("\r\nLE>"));
   }
}

   /* The following function handles the sleep indication callbacks from*/
   /* the HCI transport.                                                */
static void Sleep_Indication_Callback(Boolean_t SleepAllowed, unsigned long CallbackParameter)
{
   /* Verify parameters.                                                */
   if(CallbackParameter)
   {
      if(SleepAllowed)
      {
         /* Attempt to suspend the transport.                           */
         HCITR_COMSuspend(1);
      }
   }
}


   /* The following Toggles an LED at a passed in blink rate.           */
static void *ToggleLED(void *UserParameter)
{
   int BlinkRate = (int)UserParameter;
   Boolean_t Done;

   Done = FALSE;
   while(!Done)
   {
      HAL_LedToggle(0);

      BTPS_Delay(BlinkRate);
   }

   return(NULL);
}


   /* The following function is the main user interface thread.  It     */
   /* opens the Bluetooth Stack and then drives the main user interface.*/
static void MainThread(void *UserParameter)
{
   int                           Result;
   BTPS_Initialization_t         BTPS_Initialization;
   HCI_DriverInformation_t       HCI_DriverInformation;
   HCI_HCILLConfiguration_t      HCILLConfig;
   HCI_Driver_Reconfigure_Data_t DriverReconfigureData;

   /* Configure the UART Parameters.                                    */
   HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, 1, VENDOR_BAUD_RATE, cpHCILL_RTS_CTS);
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

        BTPS_CreateThread(ToggleLED, 116, NULL);

      /* Loop forever and process UART characters.                      */
      while(1)
      {
         ProcessCharacters(NULL);

         BTPS_Delay(200);
      }
   }
}


   /* The following is the Main application entry point.  This function */
   /* will configure the hardware and initialize the OS Abstraction     */
   /* layer, create the Main application thread and start the scheduler.*/
int main(void)
{

   void *Handle;
   Boolean_t Done;

   HAL_ConfigureHardware();

   /* Create the Main Application Thread.                               */
   if(xTaskCreate(MainThread, "MainApp", 512, NULL, 3, &Handle) != pdPASS)
   {
      /* could not create the Main Task.                                */
      while(1)
      {
         HAL_LedToggle(0);
         BTPS_Delay(100);
      }
   }

   /* Start the Task Scheduler.                                         */
   vTaskStartScheduler();

   Done = FALSE;
   while(!Done)
   {
      HAL_LedToggle(0);
      BTPS_Delay(100);
   }

   return(0);
}


