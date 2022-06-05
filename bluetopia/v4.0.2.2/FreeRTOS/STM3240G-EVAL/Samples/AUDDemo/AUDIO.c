/*****< audio.c >*************************************************************/
/*      Copyright 2013 Stonestreet One.                                      */
/*      All Rights Reserved.                                                 */
/*                                                                            */
/*      Copyright 2015 Texas Instruments Incorporated.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  AUDIO - I2C and I2S interface for audio CODEC.                           */
/*                                                                           */
/*  Author:  Marcus Funk                                                     */
/*                                                                           */
/*** MODIFICATION HISTORY ****************************************************/
/*                                                                           */
/*   mm/dd/yy  F. Lastname    Description of Modification                    */
/*   --------  -----------    -----------------------------------------------*/
/*   10/01/13  M. Funk        Initial creation.                              */
/*   21/01/15  Doron Keren    Bug fixes.                                     */
/*****************************************************************************/

/* Library includes. */
#include "BTPSKRNL.h"
#include "AUDIO.h"
#include "AUDIOCFG.h"

#include "stm324xg_eval_ioe.h"

#define AUDIO_DEFAULT_VOLUME              75

#define AUDIO_BUFFER_SIZE_WORDS           1024
#define AUDIO_I2S_DMA_LENGTH              256

#define AUDIO_CODEC_I2C_ADDRESS           0x94

#define AUDIO_INTERRUPT_PRIORITY          (1)

#define AUDIO_I2C_SHORT_RETRY_COUNT       0x00001000
#define AUDIO_I2C_LONG_RETRY_COUNT        0x0012C000

#define DEFAULT_VOLUME                    0xAF

 /* The following is used as a printf() replacement.        */
#define Display(_x)                 do { BTPS_OutputMessage _x; } while(0)

typedef enum
{
   psStopped,
   psPlaying,
   psPaused
} PlaybackState_t;

typedef struct _tagFrequencyConfiguration_t
{
   unsigned long  Frequency;
   unsigned long  PLL_I2S;
   unsigned short I2SPR;
} FrequencyConfiguration_t;

typedef struct _tagAudio_Context_t
{
   Boolean_t        Initialized;
   PlaybackState_t  PlaybackState;
   unsigned short   CurrentVolume;

   unsigned short   TxInIndex;
   unsigned short   TxOutIndex;
   unsigned short   TxWordsFree;
   unsigned short   TxBuffer[AUDIO_BUFFER_SIZE_WORDS];
} AUDIO_Context_t;

static BTPSCONST FrequencyConfiguration_t FrequencyConfiguration[] =
{
  {44100, (2 << 28) | (429 << 6), (1 << 8) | 9},
  {48000, (2 << 28) | (344 << 6), (0 << 8) | 7},
};

#define FREQENCY_CONFIGURATION_COUNT      (sizeof(FrequencyConfiguration) / sizeof(FrequencyConfiguration_t))

static BTPSCONST GPIO_InitTypeDef I2C_SDA_GpioConfiguration = {(1 << AUDIO_I2C_SDA_PIN), GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_OD, GPIO_PuPd_NOPULL};
static BTPSCONST GPIO_InitTypeDef I2C_SCL_GpioConfiguration = {(1 << AUDIO_I2C_SCL_PIN), GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_OD, GPIO_PuPd_NOPULL};

static BTPSCONST GPIO_InitTypeDef I2S_WS_GpioConfiguration  = {(1 << AUDIO_I2S_WS_PIN),  GPIO_Mode_AF, GPIO_Speed_25MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL};
static BTPSCONST GPIO_InitTypeDef I2S_SCK_GpioConfiguration = {(1 << AUDIO_I2S_SCK_PIN), GPIO_Mode_AF, GPIO_Speed_25MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL};
static BTPSCONST GPIO_InitTypeDef I2S_SDO_GpioConfiguration = {(1 << AUDIO_I2S_SDO_PIN), GPIO_Mode_AF, GPIO_Speed_25MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL};
static BTPSCONST GPIO_InitTypeDef I2S_MCK_GpioConfiguration = {(1 << AUDIO_I2S_MCK_PIN), GPIO_Mode_AF, GPIO_Speed_25MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL};

static BTPSCONST I2C_InitTypeDef  I2C_Configuration         = {100000, I2C_Mode_I2C, I2C_DutyCycle_2, 0x33, I2C_Ack_Enable, I2C_AcknowledgedAddress_7bit};

static BTPSCONST I2S_InitTypeDef  I2S_Configuration         = {I2S_Mode_MasterTx, I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_MCLKOutput_Enable, 44100, I2S_CPOL_Low};

static AUDIO_Context_t AUDIO_Context;

static int I2C_WaitEvent(unsigned int I2CEvent, ErrorStatus Value, unsigned int RetryCount);
static int I2C_WaitFlag(unsigned int I2CFlag, FlagStatus Value, unsigned int RetryCount);
static int I2C_StartTransaction(Boolean_t Transmit);
static int WriteRegister(unsigned char Address, unsigned char Value);
#if 0
static int ReadRegister(unsigned char Address);
#endif

static int SetFrequency(unsigned long SampleFrequency);
static int SetVolume(unsigned int Volume);

   /* The following function waits while an event is not in the         */
   /* specified state.  It accepts as its parameters the I2C event the  */
   /* function will monitor, the state of the event that indicates the  */
   /* wait is complete and the number of retries before the wait times  */
   /* out.  This function will return zero if succesful or a negative   */
   /* value if there was an error.                                      */
static int I2C_WaitEvent(unsigned int I2CEvent, ErrorStatus Value, unsigned int RetryCount)
{
   int ret_val;

   while((I2C_CheckEvent(AUDIO_I2C_BASE, I2CEvent) != Value) && (RetryCount))
      RetryCount --;

   /* If the retry count reached zero, indicate a failure.              */
   if(RetryCount)
      ret_val = 0;
   else
      ret_val = AUDIO_ERROR_I2C_OPERATION_FAILED;

   return(ret_val);
}

   /* The following function waits while a flag is not in the specified */
   /* state.  It accepts as its parameters the I2C flag the function    */
   /* will monitor, the state of the flag that indicates the wait is    */
   /* complete, and the number of retries before the wait times out.    */
   /* This function will return zero if succesful or a negative value if*/
   /* there was an error.                                               */
static int I2C_WaitFlag(unsigned int I2CFlag, FlagStatus Value, unsigned int RetryCount)
{
   int ret_val;

   while((I2C_GetFlagStatus(AUDIO_I2C_BASE, I2CFlag) != Value) && (RetryCount))
      RetryCount --;

   /* If the retry count reached zero, indicate a failure.              */
   if(RetryCount)
      ret_val = 0;
   else
      ret_val = AUDIO_ERROR_I2C_OPERATION_FAILED;

   return(ret_val);
}

   /* The following function starts an I2C transaction by sending the   */
   /* start operation and address.  It accepts as its paraemter a flag  */
   /* indicating if this is a transmit (TRUE) or receive (FALSE)        */
   /* operation being started.  This function returns zero if successful*/
   /* or a negative value if there was an error.                        */
static int I2C_StartTransaction(Boolean_t Transmit)
{
   int ret_val;
   /* Start the configuration sequence.                                 */
   I2C_GenerateSTART(AUDIO_I2C_BASE, ENABLE);

   if((ret_val = I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT, SUCCESS, AUDIO_I2C_SHORT_RETRY_COUNT)) == 0)
   {
      /* Transmit the slave address and enable writing operation        */
      I2C_Send7bitAddress(AUDIO_I2C_BASE, AUDIO_CODEC_I2C_ADDRESS, (Transmit) ? I2C_Direction_Transmitter : I2C_Direction_Receiver);
   }
   else
      ret_val = AUDIO_ERROR_I2C_OPERATION_FAILED;

   return(ret_val);
}

   /* The following function writes a value to the CODECs register.  It */
   /* accepts as its parameter the address of the CODEC's register and  */
   /* the value to be written.  This function returns zero if successful*/
   /* or a negative value if there was an error.                        */
static int WriteRegister(unsigned char Address, unsigned char Value)
{
   int ret_val;

   /* Wait while the I2C is busy.                                       */
   if((ret_val = I2C_WaitFlag(I2C_FLAG_BUSY, RESET, AUDIO_I2C_LONG_RETRY_COUNT)) == 0)
   {
      if((ret_val = I2C_StartTransaction(TRUE)) == 0)
      {
         if((ret_val = I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, SUCCESS, AUDIO_I2C_SHORT_RETRY_COUNT)) == 0)
         {
            /* Transmit the register's address.                         */
            I2C_SendData(AUDIO_I2C_BASE, Address);

            if((ret_val = I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING, SUCCESS, AUDIO_I2C_SHORT_RETRY_COUNT)) == 0)
            {
               /* Send the register's value.                            */
               I2C_SendData(AUDIO_I2C_BASE, Value);

               if((ret_val = I2C_WaitFlag(I2C_FLAG_BTF, SET, AUDIO_I2C_LONG_RETRY_COUNT)) == 0)
               {
                  /* Send the stop sequence.                            */
                  I2C_GenerateSTOP(AUDIO_I2C_BASE, ENABLE);
               }
            }
         }
      }
   }

   /* Return the byte read from Codec                                   */
   return ret_val;
}

#if 0
   /* The following function reads the value for the CODECs register.   */
   /* It accepts as its parameter the address of the CODEC's register to*/
   /* read.  This function returns the value of the register if         */
   /* successful or a negative value if there was an error.             */
static int ReadRegister(unsigned char Address)
{
   int ret_val;
   int RetriesRemaining;

   /* Wait while the I2C is busy.                                       */
   if((ret_val = I2C_WaitFlag(I2C_FLAG_BUSY, RESET, AUDIO_I2C_LONG_RETRY_COUNT)) == 0)
   {
      if((ret_val = I2C_StartTransaction(TRUE)) == 0)
      {
         if((ret_val = I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, SUCCESS, AUDIO_I2C_SHORT_RETRY_COUNT)) == 0)
         {
            /* Transmit the register's address.                         */
            I2C_SendData(AUDIO_I2C_BASE, Address);

            if((ret_val = I2C_WaitFlag(I2C_FLAG_BTF, SET, AUDIO_I2C_SHORT_RETRY_COUNT)) == 0)
            {
               if((ret_val = I2C_StartTransaction(FALSE)) == 0)
               {
                  if((ret_val = I2C_WaitFlag(I2C_FLAG_ADDR, SET, AUDIO_I2C_SHORT_RETRY_COUNT)) == 0)
                  {
                     /* Disable Acknowledgement.                        */
                     I2C_AcknowledgeConfig(AUDIO_I2C_BASE, DISABLE);

                     /* Clear ADDR register by reading SR1 then SR2     */
                     /* register (SR1 has already been read)            */
                     (void)AUDIO_I2C_BASE->SR2;

                     /* Send STOP Condition.                            */
                     I2C_GenerateSTOP(AUDIO_I2C_BASE, ENABLE);

                     if((ret_val = I2C_WaitFlag(I2C_FLAG_RXNE, SET, AUDIO_I2C_SHORT_RETRY_COUNT)) == 0)
                     {
                        /* Read the byte received from the Codec.       */
                        ret_val = I2C_ReceiveData(AUDIO_I2C_BASE);

                        /* Wait to make sure that STOP flag has been    */
                        /* cleared.                                     */
                        RetriesRemaining = AUDIO_I2C_SHORT_RETRY_COUNT;
                        while((AUDIO_I2C_BASE->CR1 & I2C_CR1_STOP) && (RetriesRemaining))
                           RetriesRemaining --;
                     }

                     /* Re-Enable Acknowledgements.                     */
                     I2C_AcknowledgeConfig(AUDIO_I2C_BASE, ENABLE);

                     /* Clear AF flag.                                  */
                     I2C_ClearFlag(AUDIO_I2C_BASE, I2C_FLAG_AF);
                  }
               }
            }
         }
      }
   }

   return(ret_val);
}
#endif

   /* The following function configures the I2S PLL and prescaler based */
   /* on the provided sample frequency.  This function returns zero if  */
   /* successful or a negative value if there was an error.             */
static int SetFrequency(unsigned long SampleFrequency)
{
   int          ret_val;
   unsigned int Index;

   /* Search for the frequency in the configuration list.               */
   for(Index = 0; (Index < FREQENCY_CONFIGURATION_COUNT) && (FrequencyConfiguration[Index].Frequency != SampleFrequency); Index ++);

   if(Index < FREQENCY_CONFIGURATION_COUNT)
   {
      /* Frequency Found, reconfigure the PLL.                          */

      /* Disable the PLL.                                               */
      RCC->CFGR |= RCC_CFGR_I2SSRC;
      RCC->CR   &= ~((uint32_t)RCC_CR_PLLI2SON);

      /* Set PLL as I2S clock source.                                   */
      RCC->CFGR &= ~RCC_CFGR_I2SSRC;

      /* Configure The PLL.                                             */
      RCC->PLLI2SCFGR = FrequencyConfiguration[Index].PLL_I2S;

      /* Re-enable the PLL and wait for it to be ready.                 */
      RCC->CR |= ((uint32_t)RCC_CR_PLLI2SON);

      while(!(RCC->CR & RCC_CR_PLLI2SRDY));

      /* Configure the pre-scaler.                                      */
      AUDIO_I2S_BASE->I2SPR = SPI_I2SPR_MCKOE | FrequencyConfiguration[Index].I2SPR;

      ret_val = 0;
   }
   else
      ret_val = AUDIO_ERROR_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function sets the volume of the codec to the        */
   /* specified value.  This function returns zero if successful or a   */
   /* negative value if there was an error.                             */
static int SetVolume(unsigned int Volume)
{
   int           ret_val;
   unsigned char RegisterValue;

   /* Determine the value that will be written to the CODEC's registers */
   /* for the specified volume.                                         */
   RegisterValue = (Volume * 255) / 100;

   if(RegisterValue > 0xE6)
      RegisterValue -= 0xE7;
   else
      RegisterValue += 0x19;

   /* Write the value to the registers.                                 */
   if((ret_val = WriteRegister(0x20, RegisterValue)) == 0)
      ret_val = WriteRegister(0x21, RegisterValue);

   /* Update the context information if there wasn't an error.          */
   if(!ret_val)
      AUDIO_Context.CurrentVolume = Volume;

   return(ret_val);
}

   /* The following function is used to initailize the CODEC interface. */
   /* It accepts as its parameter the sample Frequency to initailize to.*/
   /* This function returns zero if successful or a negative value if   */
   /* there was an error.                                               */
int Initialize_AUDIO(unsigned long Frequency)
{
   int             ret_val;
   I2S_InitTypeDef I2SConfig;

   if(!AUDIO_Context.Initialized)
   {
      BTPS_MemInitialize(&AUDIO_Context, 0, sizeof(AUDIO_Context_t));

      AUDIO_Context.PlaybackState = psStopped;

      /* Configure the IO Expander for the CODEC's reset pin.           */
      IOE_Config();

      /* Enable the peripheral clocks.                                  */
      RCC_APB1PeriphClockCmd(AUDIO_I2C_RCC_PERIPH_CLK_BIT | AUDIO_I2S_RCC_PERIPH_CLK_BIT, ENABLE);
      RCC_AHB1PeriphClockCmd(AUDIO_I2C_SDA_GPIO_AHB_BIT | AUDIO_I2C_SCL_GPIO_AHB_BIT | AUDIO_I2S_WS_GPIO_AHB_BIT | AUDIO_I2S_SCK_GPIO_AHB_BIT | AUDIO_I2S_SDO_GPIO_AHB_BIT | AUDIO_I2S_MCK_GPIO_AHB_BIT, ENABLE);

      /* Configure the GPIO.                                            */
      GPIO_Init(AUDIO_I2C_SDA_GPIO_PORT, (GPIO_InitTypeDef *)&I2C_SDA_GpioConfiguration);
      GPIO_Init(AUDIO_I2C_SCL_GPIO_PORT, (GPIO_InitTypeDef *)&I2C_SCL_GpioConfiguration);

      GPIO_Init(AUDIO_I2S_WS_GPIO_PORT,  (GPIO_InitTypeDef *)&I2S_WS_GpioConfiguration);
      GPIO_Init(AUDIO_I2S_SCK_GPIO_PORT, (GPIO_InitTypeDef *)&I2S_SCK_GpioConfiguration);
      GPIO_Init(AUDIO_I2S_SDO_GPIO_PORT, (GPIO_InitTypeDef *)&I2S_SDO_GpioConfiguration);
      GPIO_Init(AUDIO_I2S_MCK_GPIO_PORT, (GPIO_InitTypeDef *)&I2S_MCK_GpioConfiguration);

      GPIO_PinAFConfig(AUDIO_I2C_SDA_GPIO_PORT, AUDIO_I2C_SDA_PIN, AUDIO_I2C_GPIO_AF);
      GPIO_PinAFConfig(AUDIO_I2C_SCL_GPIO_PORT, AUDIO_I2C_SCL_PIN, AUDIO_I2C_GPIO_AF);

      GPIO_PinAFConfig(AUDIO_I2S_WS_GPIO_PORT,  AUDIO_I2S_WS_PIN,  AUDIO_I2S_GPIO_AF);
      GPIO_PinAFConfig(AUDIO_I2S_SCK_GPIO_PORT, AUDIO_I2S_SCK_PIN, AUDIO_I2S_GPIO_AF);
      GPIO_PinAFConfig(AUDIO_I2S_SDO_GPIO_PORT, AUDIO_I2S_SDO_PIN, AUDIO_I2S_GPIO_AF);
      GPIO_PinAFConfig(AUDIO_I2S_MCK_GPIO_PORT, AUDIO_I2S_MCK_PIN, AUDIO_I2S_GPIO_AF);

      /* Reset the CODEC.                                               */
      IOE_WriteIOPin(AUDIO_RESET_PIN, BitReset);
      BTPS_Delay(50);
      IOE_WriteIOPin(AUDIO_RESET_PIN, BitSet);

      /* Initialize the I2C Interface.                                  */
      I2C_Cmd(AUDIO_I2C_BASE, ENABLE);
      I2C_Init(AUDIO_I2C_BASE, (I2C_InitTypeDef *)&I2C_Configuration);

      /* Keep Codec powered OFF                                         */
      if((ret_val = WriteRegister(0x02, 0x01)) == 0)
      {
         /* Set output to Headphone.                                    */
         if((ret_val = WriteRegister(0x04, DEFAULT_VOLUME)) == 0)
         {
            /* Clock configuration: Auto detection                      */
            if((ret_val = WriteRegister(0x05, 0x81)) == 0)
            {
               /* Set the Slave Mode and the audio standard to Phillips.*/
               if((ret_val = WriteRegister(0x06, 0x04)) == 0)
               {
                  /* Set the Master volume                              */
                  if((ret_val = AUDIO_Set_Volume(AUDIO_DEFAULT_VOLUME)) == 0)
                  {
                     /* Power on the Codec                              */
                     ret_val = WriteRegister(0x02, 0x9E);
                  }
               }
            }
         }
      }

      if(!ret_val)
      {
         /* These configurations are done to reduce the time needed for */
         /* the Codec to power off.  If these configurations are        */
         /* removed, then a long delay should be added between powering */
         /* off the Codec and switching off the I2S peripheral MCLK     */
         /* clock (which is the operating clock for Codec).  If this    */
         /* delay is not inserted, then the codec will not shut down    */
         /* propoerly and it results in high noise after shut down.     */

         /* Disable the analog soft ramp                                */
         if((ret_val = WriteRegister(0x0A, 0x00)) == 0)
         {
            /* Disable the digital soft ramp                            */
            if((ret_val = WriteRegister(0x0E, 0x04)) == 0)
            {
               /* Disable the limiter attack level                      */
               if((ret_val = WriteRegister(0x27, 0x00)) == 0)
               {
                  /* Adjust Bass and Treble levels                      */
                  if((ret_val = WriteRegister(0x1F, 0x0F)) == 0)
                  {
                     /* Adjust PCM volume level                         */
                     if((ret_val = WriteRegister(0x1A, 0x0A)) == 0)
                     {
                        Display(("\r\nAdjust PCM volume level  succeded\r\n"));
                        ret_val = WriteRegister(0x1B, 0x0A);
                  }
               }
            }
         }
      }
      }

      if(!ret_val)
      {
         /* Configure the I2S peripheral                                */
         I2S_Cmd(AUDIO_I2S_BASE, DISABLE);

         /* Configure the initial PLL values for the sake of I2S_Init.  */
         RCC->CFGR       |= RCC_CFGR_I2SSRC;
         RCC->CR         &= ~((uint32_t)RCC_CR_PLLI2SON);
         RCC->CFGR       &= ~RCC_CFGR_I2SSRC;
         RCC->PLLI2SCFGR  = FrequencyConfiguration[0].PLL_I2S;
         RCC->CR         |= ((uint32_t)RCC_CR_PLLI2SON);
         while(!(RCC->CR & RCC_CR_PLLI2SRDY));

         /* Initialize the I2S interface.                               */
         BTPS_MemCopy(&I2SConfig, &I2S_Configuration, sizeof(I2S_InitTypeDef));
         I2SConfig.I2S_AudioFreq = Frequency;
         I2S_Init(AUDIO_I2S_BASE, (I2S_InitTypeDef *)&I2S_Configuration);
         if((ret_val = SetFrequency(Frequency)) == 0)
         {
            I2S_Cmd(AUDIO_I2S_BASE, ENABLE);

            NVIC_SetPriority(AUDIO_I2S_IRQ, AUDIO_INTERRUPT_PRIORITY);
            NVIC_EnableIRQ(AUDIO_I2S_IRQ);
         }
      }

      if(ret_val)
      {
         /* there was an error in the initialization, disable the I2S      */
         /* interface.                                                     */
         I2S_Cmd(AUDIO_I2S_BASE, DISABLE);
         SPI_I2S_ITConfig(AUDIO_I2S_BASE, SPI_I2S_IT_TXE, DISABLE);
         SPI_I2S_DeInit(AUDIO_I2S_BASE);
      }
      else
      {
         AUDIO_Context.Initialized = TRUE;
         AUDIO_Context.PlaybackState = psPlaying;
      }
   }
   else
   {
      ret_val = AUDIO_Pause_Resume_CODEC();
   }

   return(ret_val);
}

   /* The following function un-initilizes the codec and disables       */
   /* playback and recording.  This function will return zero if        */
   /* successful or a negative value if there was an error.             */
int Un_Initialize_AUDIO(void)
{
   I2S_Cmd(AUDIO_I2S_BASE, DISABLE);
   Display(("\r\nUn_Initialize_AUDIO delay 250ms\r\n"));
   BTPS_Delay(250);
   SPI_I2S_ITConfig(AUDIO_I2S_BASE, SPI_I2S_IT_TXE, DISABLE);
   Display(("\r\nUn_Initialize_AUDIO delay 400ms\r\n"));
   BTPS_Delay(400);
   SPI_I2S_DeInit(AUDIO_I2S_BASE);
   /* For inialize back in PLAY after PAUSE */
   AUDIO_Context.Initialized = FALSE;
   Display(("\r\nUn_Initialize_AUDIO finished...\r\n"));
   return(0);
}

   /* The following function Mutes/Un-Mutes the HW CODEC and puts       */
   /* the CODEC in Power Save mode. This function will return zero if   */
   /* successful or a negative value if there was an error.             */
int AUDIO_Pause_Resume_CODEC(void)
{
    int ret_val = 0;
    if(psPaused == AUDIO_Context.PlaybackState)
    {
        Display(("Unmute and exit CODEC power save mode\r\n"));
        /* Just Un-mute the CODEC Headphones output */
        ret_val = WriteRegister(0x04, DEFAULT_VOLUME);
        if(ret_val)
        {
           Display(("\r\nError in WriteRegister(0x04, DEFAULT_VOLUME) \r\n"));
        }
        /* Exit the Power save mode */
        ret_val =  WriteRegister(0x02, 0x9E);
        if(ret_val)
        {
           Display(("\r\nError in WriteRegister(0x02, 0x9E) \r\n"));
        }
        
        AUDIO_Context.PlaybackState = psPlaying;
    }
    else if(psPlaying == AUDIO_Context.PlaybackState)
    {
       Display(("Mute and enter CODEC power save mode\r\n"));
       /* Just mute the CODEC  */
       ret_val = WriteRegister(0x04, 0xFF);
       if(ret_val)
       {
            Display(("\r\nError in WriteRegister(0x04, 0xFF) \r\n"));
       }
       /* Put the Codec in Power save mode */    
       ret_val = WriteRegister(0x02, 0x01);
       if(ret_val)
       {
            Display(("\r\nError in WriteRegister(0x02, 0x01) \r\n"));
       }
       
       AUDIO_Context.PlaybackState = psPaused;
    }
    else
    {
       Display(("\r\nError!!! AUDIO in unknown state, PlaybackStat=%d \r\n", AUDIO_Context.PlaybackState));
    }
    
    return ret_val;
}

   /* The following function sets the playback frequency for the AUDIO  */
   /* interface.  This function returns zero if successful or a negative*/
   /* value if there was an error.                                      */
int AUDIO_Set_Frequency(unsigned long Frequency)
{
   return(SetFrequency(Frequency));
}

   /* The following function will set the volume of the audio output.   */
   /* It accepts as its parameter the new volume as a percentage between*/
   /* zero (off) and 100.  This function return zero if successful or a */
   /* negative value if there was an error.                             */
int AUDIO_Set_Volume(unsigned int NewVolume)
{
   int ret_val;

   if(NewVolume <= 100)
      ret_val = SetVolume(NewVolume);
   else
      ret_val = AUDIO_ERROR_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function returns the current level of the audio     */
   /* volume as a percentage between zero (muted) and 100.  This        */
   /* function returns the volume if successful or a negative value if  */
   /* there was an error.                                               */
int AUDIO_Get_Volume(void)
{
   return(AUDIO_Context.CurrentVolume);
}

