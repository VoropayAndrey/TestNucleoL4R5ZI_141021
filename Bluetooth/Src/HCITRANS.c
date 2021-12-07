/*****< hcitrans.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HCITRANS - HCI Transport Layer for use with Bluetopia.                    */
/*                                                                            */
/*  Author:  Marcus Funk                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/08/12  M. Funk        Initial creation.                               */
/******************************************************************************/

#include "BTPSKRNL.h"       /* Bluetooth Kernel Prototypes/Constants.         */
#include "HCITRANS.h"       /* HCI Transport Prototypes/Constants.            */
#include "HCITRCFG.h"       /* HCI Transport configuration.                   */
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_uart.h"
#include "stm32l4xx_hal_uart_ex.h"
#include "usart.h"

#define INPUT_BUFFER_SIZE        1056
#define OUTPUT_BUFFER_SIZE       1056

   /* The following define the thresholds of free space remaining in the*/
   /* transmit buffers when flow should be turned on or off when using  */
   /* software managed flow control.                                    */
   /* * NOTE * INPUT_BUFFER_SIZE must be greater than FLOW_ON_THRESHOLD */
   /*          which must be greater than FLOW_OFF_THRESHOLD.           */
#define FLOW_OFF_THRESHOLD       16
#define FLOW_ON_THRESHOLD        32

#define FlowOff()                HCITR_RTS_GPIO_PORT->ODR |= 1 << HCITR_RTS_PIN
#define FlowOn()                 HCITR_RTS_GPIO_PORT->ODR &= ~(1 << HCITR_RTS_PIN)
#define FlowIsOn()               HCITR_RTS_GPIO_PORT->ODR & (1 << HCITR_RTS_PIN)

#define ClearReset()             HAL_GPIO_WritePin(HCITR_RESET_GPIO_PORT, (1 << HCITR_RESET_PIN), GPIO_PIN_SET)
#define SetReset()               HAL_GPIO_WritePin(HCITR_RESET_GPIO_PORT, (1 << HCITR_RESET_PIN), GPIO_PIN_RESET)

#define EnableUartPeriphClock()  	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN
#define DisableUartPeriphClock() 	RCC->APB1RSTR1 &= ~RCC_APB1ENR1_USART2EN

#define USARTEnableTXInterrupt() 	HCITR_UART_BASE->CR1 |= USART_CR1_TXEIE_TXFNFIE
#define USARTDisableTXInterrupt() 	HCITR_UART_BASE->CR1 &= ~USART_CR1_TXEIE_TXFNFIE
#define USARTEnableRXInterrupt() 	HCITR_UART_BASE->CR1 |= USART_CR1_RXNEIE_RXFNEIE
#define USARTDisableRXInterrupt() 	HCITR_UART_BASE->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE
#define USARTEnableCTSInterrupt() 	EXTI->IMR1 |= HCITR_CTS_EXTI_LINE//HCITR_UART_BASE->CR3 |= USART_CR3_CTSIE;
#define USARTDisableCTSInterrupt()	EXTI->IMR1 &= ~HCITR_CTS_EXTI_LINE//HCITR_UART_BASE->CR3 &= ~USART_CR3_CTSIE;

#define DisableInterrupts()      __set_PRIMASK(1)
#define EnableInterrupts()       __set_PRIMASK(0)

#define INTERRUPT_PRIORITY       5

#define TRANSPORT_ID             1

#define DEBUG_PRINT              BTPS_OutputMessage

typedef enum
{
   hssNormal,
   hssSuspendWait,
   hssSuspendWaitInterrupted,
   hssSuspended
} SuspendState_t;

typedef struct _tagUartContext_t
{
#ifdef HCITR_ENABLE_DEBUG_LOGGING

   Boolean_t                DebugEnabled;

#endif

   SuspendState_t           SuspendState;

   HCITR_COMDataCallback_t  COMDataCallbackFunction;
   unsigned long            COMDataCallbackParameter;

   unsigned short           RxInIndex;
   unsigned short           RxOutIndex;
   volatile unsigned short  RxBytesFree;
   unsigned char            RxBuffer[INPUT_BUFFER_SIZE];

   unsigned short           TxInIndex;
   unsigned short           TxOutIndex;
   volatile unsigned short  TxBytesFree;
   unsigned char            TxBuffer[OUTPUT_BUFFER_SIZE];
} UartContext_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */

static UartContext_t              UartContext;
static int                        HCITransportOpen        = 0;

   /* Local Function Prototypes.                                        */
//static void SetBaudRate(USART_TypeDef *UartBase, unsigned int BaudRate);
//static void ConfigureGPIO(GPIO_TypeDef *Port, unsigned int Pin, GPIOMode_TypeDef Mode);
//static void SetSuspendGPIO(Boolean_t Suspend);
static void TxInterrupt(void);
static void RxInterrupt(void);

   /* The following function will reconfigure the BAUD rate without     */
   /* reconfiguring the entire port.  This function is also potentially */
   /* more accurate than the method used in the ST standard peripheral  */
   /* libraries.                                                        */
static void SetBaudRate(USART_TypeDef *UartBase, unsigned int BaudRate)
{
}

static void SetSuspendGPIO(Boolean_t Suspend)
{
}

   /* The following function is the FIFO Primer and Interrupt Service   */
   /* Routine for the UART TX interrupt.                                */
static void TxInterrupt(void)
{
   /* Continue to transmit characters as long as there is data in the   */
   /* buffer and the transmit fifo is empty.                            */
	// while
   if((UartContext.TxBytesFree != OUTPUT_BUFFER_SIZE))
   {
      /* Place the next character into the output buffer.               */
      HCITR_UART_BASE->TDR = UartContext.TxBuffer[UartContext.TxOutIndex];
      //HAL_UART_Transmit_IT(&huart2, &UartContext.TxBuffer[UartContext.TxOutIndex], 1);
      //printHex(UartContext.TxBuffer[UartContext.TxOutIndex], 1);
      //printString("\n");
      //printString("WriteDR\n");
      //printHex(UartContext.TxBuffer[UartContext.TxOutIndex], 1);
      //printString("\n");

      /* Adjust the character counts and wrap the index if necessary.   */
      UartContext.TxBytesFree++;
      UartContext.TxOutIndex++;
      if(UartContext.TxOutIndex == OUTPUT_BUFFER_SIZE) {
         UartContext.TxOutIndex = 0;
      }
   }

   /* If there are no more bytes in the queue then disable the transmit */
   /* interrupt.                                                        */
   if(UartContext.TxBytesFree == OUTPUT_BUFFER_SIZE) {
	   USARTDisableTXInterrupt();
   }
}

   /* The following function is the Interrupt Service Routine for the   */
   /* UART RX interrupt.                                                */
static void RxInterrupt(void)
{
   /* Continue reading data from the fifo until it is empty or the      */
   /* buffer is full.                                                   */
	// while
   if((UartContext.RxBytesFree))
   {
      if(HCITR_UART_BASE->ISR & USART_ISR_ORE) {
         DBG_MSG(DBG_ZONE_GENERAL, ("Receive Overflow\r\n"));
      }

      /* Read a character from the port into the receive buffer         */
      UartContext.RxBuffer[UartContext.RxInIndex] = HCITR_UART_BASE->RDR;
      //HAL_UART_Receive_IT(&huart2, &UartContext.RxBuffer[UartContext.RxInIndex], 1);
      //printHex(UartContext.RxBuffer[UartContext.RxInIndex], 1);
      //printString("\n");
      /* Update the count variables.                                    */
      UartContext.RxBytesFree--;
      UartContext.RxInIndex++;
      if(UartContext.RxInIndex == INPUT_BUFFER_SIZE) {
         UartContext.RxInIndex = 0;
      }
   }

  /* If the buffer is full, disable the receive interrupt.          */
  if(!UartContext.RxBytesFree) {
	  FlowOff();
	  USARTDisableRXInterrupt();
  }

   if(UartContext.SuspendState == hssSuspendWait)
   {
      /* Indicate the suspend is interrupted.                           */
      UartContext.SuspendState = hssSuspendWaitInterrupted;
   }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if(GPIO_Pin == GPIO_PIN_3) {
		//EXTI->PR |= EXTI_PR_PR9;
		//printString("CTS\n");
		if(UartContext.SuspendState == hssSuspended) {
		  /* Resume the UART.                                               */
		  EnableUartPeriphClock();
		  SetSuspendGPIO(FALSE);
		  UartContext.SuspendState = hssNormal;
		} else {
		  if(UartContext.SuspendState == hssSuspendWait) {
			 /* Indicate the suspend is interrupted.                        */
			 UartContext.SuspendState = hssSuspendWaitInterrupted;
		  }
		}
		/* Enable the UART transmit interrupt if there is data in the buffer.*/
		if(UartContext.TxBytesFree != OUTPUT_BUFFER_SIZE) {
		   USARTEnableTXInterrupt();
		   //TxInterrupt();
		   //printString("USARTEnableTXInterrupt\n");
		}
		USARTEnableRXInterrupt();
		FlowOn();
	}
	/*
	printString("CR1:");
	printHex(HCITR_UART_BASE->CR1, 2);
	printString("\nCR2:");
	printHex(HCITR_UART_BASE->CR2, 2);
	printString("\nCR3:");
	printHex(HCITR_UART_BASE->CR3, 2);
	printString("\n");
	*/
}

void HCITR_UART_IRQ_HANDLER(void)
{
	unsigned int Flags;

	//unsigned int Control;

	Flags   = HCITR_UART_BASE->ISR;
	//Control = HCITR_UART_BASE->CR1;

	/* Check to see if data is available in the Receive Buffer.          */
	//if((Flags & (USART_SR_RXNE | USART_SR_ORE))) {
	if((Flags & USART_ISR_TXE_TXFNF)) {
		//HCITR_UART_BASE->SR &= ~USART_SR_TC;
		//printString("TXE\n");
		TxInterrupt();
	}


	if((Flags & (USART_ISR_RXNE_RXFNE | USART_ISR_ORE))) {
		//printString("RXE\n");
		RxInterrupt();
		HCITR_UART_BASE->ISR &= ~USART_ISR_RXNE_RXFNE;
		HCITR_UART_BASE->ISR &= ~USART_ISR_ORE;
	}

	if((Flags & USART_ISR_NE)) {
		//printString("NE\n");
		HCITR_UART_BASE->ISR &= ~USART_ISR_NE;
	}

	if((Flags & USART_ISR_FE)) {
		//printString("FE\n");
		HCITR_UART_BASE->ISR &= ~USART_ISR_FE;
	}
}

   /* The following function is responsible for opening the HCI         */
   /* Transport layer that will be used by Bluetopia to send and receive*/
   /* COM (Serial) data.  This function must be successfully issued in  */
   /* order for Bluetopia to function.  This function accepts as its    */
   /* parameter the HCI COM Transport COM Information that is to be used*/
   /* to open the port.  The final two parameters specify the HCI       */
   /* Transport Data Callback and Callback Parameter (respectively) that*/
   /* is to be called when data is received from the UART.  A successful*/
   /* call to this function will return a non-zero, positive value which*/
   /* specifies the HCITransportID that is used with the remaining      */
   /* transport functions in this module.  This function returns a      */
   /* negative return value to signify an error.                        */
int BTPSAPI HCITR_COMOpen(HCI_COMMDriverInformation_t *COMMDriverInformation, HCITR_COMDataCallback_t COMDataCallback, unsigned long CallbackParameter)
{
   int ret_val;
   //printString("HCITR_COMOpen\n");
   /* First, make sure that the port is not already open and make sure  */
   /* that valid COMM Driver Information was specified.                 */
   if((!HCITransportOpen) && (COMMDriverInformation) && (COMDataCallback))
   {
      /* Initialize the return value for success.                       */
      ret_val                              = TRANSPORT_ID;

      /* Flag that the HCI Transport is open.                           */
      HCITransportOpen                     = 1;

      /* Initialize the context structure.                              */
      BTPS_MemInitialize(&UartContext, 0, sizeof(UartContext_t));

      UartContext.COMDataCallbackFunction  = COMDataCallback;
      UartContext.COMDataCallbackParameter = CallbackParameter;
      UartContext.TxBytesFree              = OUTPUT_BUFFER_SIZE;
      UartContext.RxBytesFree              = INPUT_BUFFER_SIZE;
      UartContext.SuspendState             = hssNormal;
      //UartContext.DebugEnabled				= ENABLE;


      /* Enable the peripheral clocks for the UART and its GPIO.        */
      EnableUartPeriphClock();
      USARTDisableTXInterrupt();
      USARTDisableCTSInterrupt();

      //__HAL_UART_CLEAR_FLAG(&huart2, (UART_CLEAR_TCF | UART_CLEAR_TXFECF));

      MX_USART2_UART_Init();
      USARTEnableRXInterrupt();
      FlowOff();


      SetReset();
      /* Clear the reset.                                               */
      BTPS_Delay(20);
      //EXTI->PR |= EXTI_PR_PR9;
      USARTEnableCTSInterrupt();
      USARTEnableRXInterrupt();
      USARTEnableTXInterrupt();

      //printString("CTS enable\n");
      ClearReset();
      BTPS_Delay(250);

   } else {
      ret_val = HCITR_ERROR_UNABLE_TO_OPEN_TRANSPORT;
   }

   return(ret_val);
}

   /* The following function is responsible for closing the specific HCI*/
   /* Transport layer that was opened via a successful call to the      */
   /* HCITR_COMOpen() function (specified by the first parameter).      */
   /* Bluetopia makes a call to this function whenever an either        */
   /* Bluetopia is closed, or an error occurs during initialization and */
   /* the driver has been opened (and ONLY in this case).  Once this    */
   /* function completes, the transport layer that was closed will no   */
   /* longer process received data until the transport layer is         */
   /* Re-Opened by calling the HCITR_COMOpen() function.                */
   /* * NOTE * This function *MUST* close the specified COM Port.  This */
   /*          module will then call the registered COM Data Callback   */
   /*          function with zero as the data length and NULL as the    */
   /*          data pointer.  This will signify to the HCI Driver that  */
   /*          this module is completely finished with the port and     */
   /*          information and (more importantly) that NO further data  */
   /*          callbacks will be issued.  In other words the very last  */
   /*          data callback that is issued from this module *MUST* be a*/
   /*          data callback specifying zero and NULL for the data      */
   /*          length and data buffer (respectively).                   */
void BTPSAPI HCITR_COMClose(unsigned int HCITransportID)
{
   HCITR_COMDataCallback_t COMDataCallback;
//   printString("HCITR_COMClose\n");
#if (defined(SUPPORT_TRANSPORT_SUSPEND) || defined(USE_SOFTWARE_CTS_RTS))

  // EXTI_InitTypeDef ExtIntConfiguration;

#endif

   /* Check to make sure that the specified Transport ID is valid.      */
   if((HCITransportID == TRANSPORT_ID) && (HCITransportOpen))
   {
      /* Flag that the HCI Transport is no longer open.                 */
      HCITransportOpen = 0;

#if (defined(SUPPORT_TRANSPORT_SUSPEND) || defined(USE_SOFTWARE_CTS_RTS))

      /* Disable external interrupt for the CTS line                    */
    //  USARTDisableCTSInterrupt();
    //  NVIC_DisableIRQ(HCITR_CTS_IRQ);
    //  BTPS_MemCopy(&ExtIntConfiguration, &CTS_ExtIntConfiguration, sizeof(EXTI_InitTypeDef));
    //  ExtIntConfiguration.EXTI_LineCmd = DISABLE;
    //  EXTI_Init(&ExtIntConfiguration);

#endif

      NVIC_DisableIRQ(HCITR_UART_IRQ);

      /* Appears to be valid, go ahead and close the port.              */
      USARTDisableRXInterrupt();
      USARTDisableTXInterrupt();
      USARTDisableCTSInterrupt();
      FlowOff();
      //USART_ITConfig(HCITR_UART_BASE, USART_IT_RXNE, DISABLE);
      //USART_ITConfig(HCITR_UART_BASE, USART_IT_TXE,  DISABLE);

      /* Place the Bluetooth Device in Reset.                           */
      SetReset();

      /* Disable the peripheral clock for the UART.                     */
      DisableUartPeriphClock();

      /* Note the Callback information.                                 */
      COMDataCallback   = UartContext.COMDataCallbackFunction;

      UartContext.COMDataCallbackFunction = NULL;

      /* All finished, perform the callback to let the upper layer know */
      /* that this module will no longer issue data callbacks and is    */
      /* completely cleaned up.                                         */
      if(COMDataCallback)
         (*COMDataCallback)(HCITransportID, 0, NULL, UartContext.COMDataCallbackParameter);

      UartContext.COMDataCallbackParameter = 0;
   }
}

   /* The following function is responsible for instructing the         */
   /* specified HCI Transport layer (first parameter) that was opened   */
   /* via a successful call to the HCITR_COMOpen() function to          */
   /* reconfigure itself with the specified information.                */
   /* * NOTE * This function does not close the HCI Transport specified */
   /*          by HCI Transport ID, it merely reconfigures the          */
   /*          transport.  This means that the HCI Transport specified  */
   /*          by HCI Transport ID is still valid until it is closed via*/
   /*          the HCI_COMClose() function.                             */
void BTPSAPI HCITR_COMReconfigure(unsigned int HCITransportID, HCI_Driver_Reconfigure_Data_t *DriverReconfigureData)
{
   HCI_COMMReconfigureInformation_t *ReconfigureInformation;
//   printString("HCITR_COMReconfigure\n");
   /* Check to make sure that the specified Transport ID is valid.      */
   if((HCITransportID == TRANSPORT_ID) && (HCITransportOpen) && (DriverReconfigureData))
   {
      if((DriverReconfigureData->ReconfigureCommand == HCI_COMM_DRIVER_RECONFIGURE_DATA_COMMAND_CHANGE_COMM_PARAMETERS) && (DriverReconfigureData->ReconfigureData))
      {
         ReconfigureInformation = (HCI_COMMReconfigureInformation_t *)(DriverReconfigureData->ReconfigureData);

         /* Check if the baud rate needs to change.                     */
         if(ReconfigureInformation->ReconfigureFlags & (HCI_COMM_RECONFIGURE_INFORMATION_RECONFIGURE_FLAGS_CHANGE_BAUDRATE | HCI_COMM_RECONFIGURE_INFORMATION_RECONFIGURE_FLAGS_CHANGE_PROTOCOL))
         {
            DisableInterrupts();
            SetBaudRate(HCITR_UART_BASE, ReconfigureInformation->BaudRate);
            EnableInterrupts();
         }
      }
   }
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to force the processing of incoming COM Data.             */
   /* * NOTE * This function is only applicable in device stacks that   */
   /*          are non-threaded.  This function has no effect for device*/
   /*          stacks that are operating in threaded environments.      */
void BTPSAPI HCITR_COMProcess(unsigned int HCITransportID)
{
   unsigned int MaxLength;
   unsigned int TotalLength;
//   printString("HCITR_COMProcess\n");
#ifdef HCITR_ENABLE_DEBUG_LOGGING

   unsigned int Index;

#endif

   /* Check to make sure that the specified Transport ID is valid.      */
   if((HCITransportID == TRANSPORT_ID) && (HCITransportOpen))
   {
      /* Loop until the receive buffer is empty.                        */
      while((TotalLength = (INPUT_BUFFER_SIZE - UartContext.RxBytesFree)) != 0)
      {
         /* Determine the maximum number of characters that we can send */
         /* before we reach the end of the buffer.  We need to process  */
         /* the smaller of the max characters or the number of          */
         /* characters that are in the buffer.                          */
         MaxLength   = (INPUT_BUFFER_SIZE - UartContext.RxOutIndex);
         if(TotalLength > MaxLength)
            TotalLength = MaxLength;

#ifdef HCITR_ENABLE_DEBUG_LOGGING

         if(UartContext.DebugEnabled)
         {
            DEBUG_PRINT(">");

            for(Index = 0; Index < TotalLength; Index ++)
               DEBUG_PRINT(" %02X", UartContext.RxBuffer[UartContext.RxOutIndex + Index]);

            DEBUG_PRINT("\r\n");
         }

#endif

         /* Call the upper layer back with the data.                    */
         if(UartContext.COMDataCallbackFunction)
            (*UartContext.COMDataCallbackFunction)(TRANSPORT_ID, TotalLength, &(UartContext.RxBuffer[UartContext.RxOutIndex]), UartContext.COMDataCallbackParameter);

         /* Adjust the Out Index and handle any looping.                */
         UartContext.RxOutIndex += TotalLength;
         if(UartContext.RxOutIndex == INPUT_BUFFER_SIZE)
            UartContext.RxOutIndex = 0;

         /* Credit the amount that was processed and make sure the      */
         /* receive interrupt is enabled.                               */
         DisableInterrupts();
         UartContext.RxBytesFree += TotalLength;
         //USART_ITConfig(HCITR_UART_BASE, USART_IT_RXNE, ENABLE);
         USARTEnableRXInterrupt();
         EnableInterrupts();
/*
#ifdef USE_SOFTWARE_CTS_RTS

         if(UartContext.SuspendState == hssNormal)
         {
            // If the input buffer has passed the flow on threshold,
            // re-enable flow control.
            if(UartContext.RxBytesFree >= FLOW_ON_THRESHOLD)
               FlowOn();
         }

#endif
 */

      }
   }
}

   /* The following function is responsible for actually sending data   */
   /* through the opened HCI Transport layer (specified by the first    */
   /* parameter).  Bluetopia uses this function to send formatted HCI   */
   /* packets to the attached Bluetooth Device.  The second parameter to*/
   /* this function specifies the number of bytes pointed to by the     */
   /* third parameter that are to be sent to the Bluetooth Device.  This*/
   /* function returns a zero if the all data was transferred           */
   /* successfully or a negative value if an error occurred.  This      */
   /* function MUST NOT return until all of the data is sent (or an     */
   /* error condition occurs).  Bluetopia WILL NOT attempt to call this */
   /* function repeatedly if data fails to be delivered.  This function */
   /* will block until it has either buffered the specified data or sent*/
   /* all of the specified data to the Bluetooth Device.                */
   /* * NOTE * The type of data (Command, ACL, SCO, etc.) is NOT passed */
   /*          to this function because it is assumed that this         */
   /*          information is contained in the Data Stream being passed */
   /*          to this function.                                        */
int BTPSAPI HCITR_COMWrite(unsigned int HCITransportID, unsigned int Length, unsigned char *Buffer)
{
	//printString("HCITR_COMWrite\n");
	//printString("Length: ");
	//printUnsignedInt(Length);
	//printString("\n");

	int ret_val;
	int Count;
	int BytesFree;

#ifdef HCITR_ENABLE_DEBUG_LOGGING

	unsigned int Index;

#endif

   /* Check to make sure that the specified Transport ID is valid and   */
   /* the output buffer appears to be valid as well.                    */
   if((HCITransportID == TRANSPORT_ID) && (HCITransportOpen) && (Length) && (Buffer))
   {
      /* If the UART is suspended, resume it.                           */
      if(UartContext.SuspendState == hssSuspended)
      {
         DisableInterrupts();

         EnableUartPeriphClock();
         SetSuspendGPIO(FALSE);
         UartContext.SuspendState = hssNormal;

         EnableInterrupts();
      }

#ifdef HCITR_ENABLE_DEBUG_LOGGING

      if(UartContext.DebugEnabled)
      {
         DEBUG_PRINT("<");
         for(Index = 0; Index < Length; Index++) {
            DEBUG_PRINT(" %02X", Buffer[Index]);
      	 }
         DEBUG_PRINT("\r\n");
      }

#endif

      /* Process all of the data.                                       */
      while(Length)
      {
         /* Wait for space in the transmit buffer.                      */
         while(!UartContext.TxBytesFree) {}

         /* The data may have to be copied in 2 phases.  Calculate the  */
         /* number of character that can be placed in the buffer before */
         /* the buffer must be wrapped.                                 */
         BytesFree = UartContext.TxBytesFree;
         Count = Length;
         // If Count bigger than BytesFree in TxBuffer only BytesFree will be copy
         Count = (BytesFree < Count) ? BytesFree : Count;
         Count = ((OUTPUT_BUFFER_SIZE - UartContext.TxInIndex) < Count) ? (OUTPUT_BUFFER_SIZE - UartContext.TxInIndex) : Count;

         //printString("Count: ");
         //printUnsignedInt(Count);
         //printString("\n");
         BTPS_MemCopy(&(UartContext.TxBuffer[UartContext.TxInIndex]), Buffer, Count);

         /* Update the number of free bytes in the buffer.  Since this  */
         /* count can also be updated in the interrupt routine, we will */
         /* have to update this with interrupts disabled.               */

         /* Adjust the index values.                                    */
         Buffer                  += Count;
         Length                  -= Count;
         UartContext.TxInIndex   += Count;
         if(UartContext.TxInIndex == OUTPUT_BUFFER_SIZE) {
            UartContext.TxInIndex = 0;
         }

         /* Update the bytes free and make sure the transmit interrupt  */
         /* is enabled.                                                 */
         DisableInterrupts();
         UartContext.TxBytesFree -= Count;
         //HCITR_UART_BASE->TDR = (UartContext.TxBuffer[UartContext.TxOutIndex]);
         USARTEnableTXInterrupt();
         USARTEnableRXInterrupt();
         EnableInterrupts();
         //printString("WriteDR\n");
      }

      ret_val = 0;
   }
   else
      ret_val = HCITR_ERROR_WRITING_TO_PORT;

   return(ret_val);
}

   /* The following function is responsible for suspending the HCI COM  */
   /* transport.  It will block until the transmit buffers are empty and*/
   /* all data has been sent then put the transport in a suspended      */
   /* state.  This function will return a value of 0 if the suspend was */
   /* successful or a negative value if there is an error.              */
   /* * NOTE * An error will occur if the suspend operation was         */
   /*          interrupted by another event, such as data being received*/
   /*          before the transmit buffer was empty.                    */
   /* * NOTE * The calling function must lock the Bluetooth stack with a*/
   /*          call to BSC_LockBluetoothStack() before this function is */
   /*          called.                                                  */
   /* * NOTE * This function should only be called when the baseband    */
   /*          low-power protocol in use has indicated that it is safe  */
   /*          to sleep.  Also, this function must be called            */
   /*          successfully before any clocks necessary for the         */
   /*          transport to operate are disabled.                       */
int BTPSAPI HCITR_COMSuspend(unsigned int HCITransportID)
{
   int ret_val;
   //printString("HCITR_COMSuspend\n");
#ifdef SUPPORT_TRANSPORT_SUSPEND

   if(HCITransportID == TRANSPORT_ID)
   {
      /* Signal that we are waiting for a suspend operation to complete.*/
      UartContext.SuspendState = hssSuspendWait;

      /* Set the CTS interrupt.                                         */
      SetSuspendGPIO(TRUE);

      /* Wait for the UART transmit buffer and FIFO to be empty.        */
      //while(((UartContext.TxBytesFree != OUTPUT_BUFFER_SIZE) || (USART_GetFlagStatus(HCITR_UART_BASE, UART_FLAG_TC) != SET)) && (UartContext.SuspendState == hssSuspendWait)) {}
      while(((UartContext.TxBytesFree != OUTPUT_BUFFER_SIZE) || (HCITR_UART_BASE->ISR & USART_ISR_TC == 0)) && (UartContext.SuspendState == hssSuspendWait)) {}


      /* Confirm that no data was received in this time and suspend the */
      /* UART.                                                          */
      DisableInterrupts();

      if(UartContext.SuspendState == hssSuspendWait)
      {
         UartContext.SuspendState = hssSuspended;

         /* Disable the UART clock.                                     */
         DisableUartPeriphClock();

         ret_val = 0;
      }
      else
      {
         /* Data was received, abort suspending the UART.               */
         SetSuspendGPIO(FALSE);

         ret_val = HCITR_ERROR_SUSPEND_ABORTED;
      }

      EnableInterrupts();

   }
   else
      ret_val = HCITR_ERROR_INVALID_PARAMETER;

#else

   ret_val = HCITR_ERROR_INVALID_PARAMETER;

#endif

   return(ret_val);
}

   /* The following function is used to enable or disable debug logging */
   /* within HCITRANS.  The function accepts as its parameter a flag    */
   /* which indicates if debugging should be enabled.  It returns zero  */
   /* if successful or a negative value if there was an error.          */
int BTPSAPI HCITR_EnableDebugLogging(Boolean_t Enable)
{
   int ret_val;

#ifdef HCITR_ENABLE_DEBUG_LOGGING

   UartContext.DebugEnabled = Enable;

   ret_val = 0;

#else

   ret_val = HCITR_ERROR_INVALID_PARAMETER;

#endif

   return(ret_val);
}

//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
//	if(huart->Instance == USART2) {
//		RxInterrupt();
//	}
//}
//
//void HAL_UARTEx_TxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
//	if(huart->Instance == USART2) {
//		TxInterrupt();
//	}
//}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance == USART2) {
			TxInterrupt();
	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance == USART2) {
			RxInterrupt();
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {

}

