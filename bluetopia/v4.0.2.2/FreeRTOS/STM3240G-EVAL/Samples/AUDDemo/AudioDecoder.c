/*****< audiodecoder.c >*******************************************************/
/*      Copyright 2011 - 2013 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*      Copyright 2015 Texas Instruments Incorporated.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/* AUDIODECODER  - Sample abstraction code to decode an SBC Audio Frame.      */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/13/11  G. Hensley     Initial creation. (Based on LinuxAUDM)          */
/******************************************************************************/

#include "SS1BTPS.h"       /* Includes for the SS1 Bluetooth Protocol Stack.  */
#include "BTPSKRNL.h"      /* BTPS Kernel Header.                             */

#include "SS1BTAUD.h"      /* Includes for the SS1 Audio Profile Sub-System.  */
#include "SS1BTA2D.h"      /* Includes for the SS1 A2DP API.                  */
#include "SS1SBC.h"        /* Includes for the SS1 Subband Codec.             */

#include "AudioDecoder.h"  /* Audio Decoder sample.                           */
#include "AUDIO.h"
#include "AUDIOCFG.h"

   /* The following enumerates the states that the playback state       */
   /* machine may be in.                                                */
typedef enum
{
   asIdle,
   asBuffering,
   asDecoding,
   asPlaying
} AudioState_t;

   /* Constants that define the size of the buffer to hold the SBC      */
   /* Frames.                                                           */
#define SBC_BUFFER_SIZE                                     (18 * 1024)

   /* The follwoing defines the number of SBC frames that will be       */
   /* processed to produce a fixed amount of Audio Data.                */
#define SBC_PROCESS_FRAME_COUNT                                      8

   /* Each SBC frame will yield 128 left samples and 128 right samples. */
   /* Setup the buffer to hold 8 decoded SBC frames.                    */
#define AUDIO_BUFFER_SIZE                                     (128 * 8)

   /* The following defines the number of audio samples that are        */
   /* generated for each SBC frame.                                     */
#define NUM_AUDIO_SAMPLES_PER_SBC_FRAME                             128

   /* The following defines a constant that is used to control the      */
   /* jitter of audio playback.  The decode routine monitors that amount*/
   /* of audio samples in the playback buffer.  If the system decodes   */
   /* samples faster than the playback then the buffer will fill.  If   */
   /* the playback is faster then the decoder then we will run out of   */
   /* samples to play.  Playback buffer space is continuously monitored */
   /* and when we have BufferHighLimit of audio samples in the buffer,  */
   /* the SAMPLE_RATE_ADJUSTMENT_VALUE is added to the current sample   */
   /* rate in order to playback faster and open up some buffer space.   */
   /* If the audio samples passes BufferLowLimit, then the playback     */
   /* speed is reduced back to normal.                                  */
#define SAMPLE_RATE_ADJUSTMENT_VALUE                                100

   /* Constant that defines the size (in bytes) of the SBC Decode       */
   /* Thread.                                                           */
#define DECODE_THREAD_STACK_SIZE                                  3072


   /* The following is used as a printf() replacement.                  */
#define Display(_x)                 do { BTPS_OutputMessage _x; } while(0)

   /* The following type definition represents the structure which holds*/
   /* the current audio playback context information.                   */
typedef struct _tagPlaybackContext
{
   unsigned int                BluetoothStackID;

   Decoder_t                   DecoderHandle;
   AUD_Stream_Configuration_t  StreamConfig;
   SBC_Decode_Configuration_t  DecodeConfiguration;

   Event_t                     SBCDecodeEvent;
   Event_t                     ShutdownEvent;
   volatile Boolean_t          Exit;

   AudioState_t                AudioState;
   unsigned int                SBCIn;
   unsigned int                SBCOut;
   unsigned int                SBCEnd;
   unsigned int                SBCFree;
   unsigned int                SBCUsed;
   unsigned int                SBCFrameLength;
   unsigned char               SBCBuffer[SBC_BUFFER_SIZE];
   unsigned short             *LeftChannelBuffer;
   unsigned short             *RightChannelBuffer;
   unsigned int                InIndex;
   unsigned int                OutIndex;
   unsigned int                EndIndex;
   Boolean_t                   LeftChannelOut;
   unsigned int                NumAudioSamples;
   unsigned int                DisplayFormatInfo;
   unsigned long               CurrentSampleRate;
   unsigned char               SampleRateAdjustment;
   unsigned int                SampleRateAdjustmentCount;
   unsigned int                BufferHighLimit;
   unsigned int                BufferLowLimit;
} PlaybackContext_t;

 void* DecoderThreadId = 0;
 void* LastDecoderThreadId = 0;

   /* Single instance of the Playback Context                           */
static PlaybackContext_t PlaybackContext;

   /* Flag to indicated if the module is initialized                    */
static Boolean_t Initialized = FALSE;

static int Decode(unsigned int DataLength, unsigned char *DataPtr);
static void *BTPSAPI PlaybackThread(void *ThreadParameter);

   /* The following function issued to decode a block of SBC data.  When*/
   /* a full buffer of data is decoded, the data is delivered to the    */
   /* upper layer for playback.                                         */
static int Decode(unsigned int DataLength, unsigned char *DataPtr)
{
   int               ret_val;
   unsigned int      UnusedDataLength;
   SBC_Decode_Data_t DecodedData;

   /* Note the amount of unprocessed data.                              */
   UnusedDataLength = DataLength;

   /* Verify that the parameters passed in appear valid.                */
   if((DataLength) && (DataPtr))
   {
      while(UnusedDataLength)
      {
         /* Initialize the Decode Data structure for this iteration.    */
         DecodedData.LeftChannelDataLength  = 0;
         DecodedData.RightChannelDataLength = 0;
//xxx The two lines below show how to set the indexes for Interleaving and non-Interleaving
//xxx This example shows Interleaving.
//xxx
//xxx Non Interleaved   DecodedData.LeftChannelDataPtr     = &(PlaybackContext.LeftChannelBuffer[PlaybackContext.InIndex]);
//xxx Non Interleaved   DecodedData.RightChannelDataPtr    = &(PlaybackContext.RightChannelBuffer[PlaybackContext.InIndex]);
//xxx Interleaved       DecodedData.LeftChannelDataPtr     = &(PlaybackContext.LeftChannelBuffer[PlaybackContext.InIndex*2]);
//xxx Interleaved       DecodedData.RightChannelDataPtr    = &(PlaybackContext.RightChannelBuffer[PlaybackContext.InIndex*2]);
//xxx
         DecodedData.LeftChannelDataPtr     = &(PlaybackContext.LeftChannelBuffer[PlaybackContext.InIndex]);
         DecodedData.RightChannelDataPtr    = &(PlaybackContext.RightChannelBuffer[PlaybackContext.InIndex]);
         DecodedData.ChannelDataSize        = (unsigned int)(AUDIO_BUFFER_SIZE-PlaybackContext.InIndex);

         /* Calculate the amount of space available for audio samples.  */
         if(PlaybackContext.OutIndex > PlaybackContext.InIndex)
            DecodedData.ChannelDataSize = (unsigned int)(PlaybackContext.OutIndex - PlaybackContext.InIndex);
         else
            DecodedData.ChannelDataSize = (unsigned int)(AUDIO_BUFFER_SIZE - PlaybackContext.InIndex);

         /* Make sure that there is enough room to contain the samples  */
         /* from a single SBC frame.                                    */
         if(DecodedData.ChannelDataSize < NUM_AUDIO_SAMPLES_PER_SBC_FRAME)
            break;

         /* Pass the SBC data into the Decoder.  If a complete Frame was*/
         /* decoded then we need to write the decoded data to the output*/
         /* file.                                                       */
         ret_val = SBC_Decode_Data(PlaybackContext.DecoderHandle, UnusedDataLength, &DataPtr[(DataLength - UnusedDataLength)], &PlaybackContext.DecodeConfiguration, &DecodedData, &UnusedDataLength);
         if(ret_val == SBC_PROCESSING_COMPLETE)
         {
            if(PlaybackContext.DisplayFormatInfo)
            {
               PlaybackContext.DisplayFormatInfo = 0;

               Display(("Frame Length     : %d\r\n", PlaybackContext.DecodeConfiguration.FrameLength));
               Display(("Bit Pool         : %d\r\n", PlaybackContext.DecodeConfiguration.BitPool));
               Display(("Bit Rate         : %d\r\n", PlaybackContext.DecodeConfiguration.BitRate));
               Display(("Buffer Length    : %d\r\n", DecodedData.LeftChannelDataLength));
               Display(("Frames/GAVD      : %d\r\n", (DataLength/PlaybackContext.DecodeConfiguration.FrameLength)));
               Display(("\r\nSink> \b"));
            }

            /* Adjust the Index for the Audio Samples that were just    */
            /* added.                                                   */
            PlaybackContext.InIndex += DecodedData.LeftChannelDataLength;

            SPI_I2S_ITConfig(AUDIO_I2S_BASE, SPI_I2S_IT_TXE, DISABLE);

            /* Check to see if we are waiting to build up samples before*/
            /* we start playing the audio.                              */
            if(PlaybackContext.AudioState == asPlaying)
            {
               /* Update the number of audio samples that are available */
               /* to be played.                                         */
               PlaybackContext.NumAudioSamples += DecodedData.LeftChannelDataLength;

               SPI_I2S_ITConfig(AUDIO_I2S_BASE, SPI_I2S_IT_TXE, ENABLE);
            }
            else
            {
               /* Check to see if we are waiting to build up samples    */
               /* before we start playing the audio.                    */
               if(PlaybackContext.AudioState == asDecoding)
               {
                  PlaybackContext.NumAudioSamples += DecodedData.LeftChannelDataLength;

                  /* Check to see if we have enough samples to start.   */
                  if(PlaybackContext.NumAudioSamples >= (AUDIO_BUFFER_SIZE >> 1))
                  {
                     /* It is time to start the audio, so enable the    */
                     /* audio interrupt.                                */
                     PlaybackContext.AudioState = asPlaying;

                     SPI_I2S_ITConfig(AUDIO_I2S_BASE, SPI_I2S_IT_TXE, ENABLE);
                  }
               }
            }

            /* Check to see if another packet would overflow the buffer.*/
            if((PlaybackContext.InIndex + DecodedData.LeftChannelDataLength) > AUDIO_BUFFER_SIZE)
            {
               PlaybackContext.EndIndex = PlaybackContext.InIndex;
               PlaybackContext.InIndex = 0;
            }
         }
         else
            break;
      }
   }

   /* Return the number of bytes that were processed.                   */
   return(DataLength - UnusedDataLength);
}

   /* The following thread function is used to process SBC data         */
   /* retrieved by the remote device.                                   */
static void *BTPSAPI PlaybackThread(void *ThreadParameter)
{
   int BytesUsed;

   Display(("Enter Decode Thread\r\n"));

   /* Verify the parameters that we require for starting the thread in the first time        */
   if((!PlaybackContext.Exit) && (PlaybackContext.SBCDecodeEvent) && (PlaybackContext.ShutdownEvent))
   {
      /* Thread never ends, just on SW reset                            */
      while(1)
      {
         /* Wait until there is something to do.                        */
         if(BTPS_WaitEvent(PlaybackContext.SBCDecodeEvent, BTPS_INFINITE_WAIT))
         {
            /* Event signalled, make sure we have not been instructed to*/
            /* exit.                                                    */
            if(!PlaybackContext.Exit)
            {
               /* If we do not have any buffers allocated to decode data*/
               /* into, then we need to allocate one.                   */
               if(!PlaybackContext.LeftChannelBuffer)
               {
                  PlaybackContext.LeftChannelBuffer  = (unsigned short *)BTPS_AllocateMemory(AUDIO_BUFFER_SIZE * sizeof(unsigned short) * 2);
//xxx The two lines below show how to set the indexes for Interleaving and non-Interleaving
//xxx This example shows Interleaving.
//xxx
//xxx Non Interleaved   PlaybackContext.RightChannelBuffer = &(PlaybackContext.LeftChannelBuffer[AUDIO_BUFFER_SIZE]);
//xxx Interleaved       PlaybackContext.RightChannelBuffer = &(PlaybackContext.LeftChannelBuffer[1]);
//xxx
                  PlaybackContext.RightChannelBuffer = &(PlaybackContext.LeftChannelBuffer[AUDIO_BUFFER_SIZE]);
               }

               PlaybackContext.EndIndex = AUDIO_BUFFER_SIZE;

               /* We have not been instructed to exit, so go ahead and  */
               /* attempt to lock the stack.                            */
               /* * NOTE * We are not going to use the actual Bluetopia */
               /*          lock because we could potentially not allow  */
               /*          it to run.  The reason is the delay that is  */
               /*          used below (timeout of zero).  By using this */
               /*          lock we can allow Bluetopia time to run      */
               /*          without worry to starve the task.            */
               if(BSC_AcquireListLock())
               {
                  /* Verify that we have audio data to process.         */
                  if((PlaybackContext.SBCUsed) && (PlaybackContext.AudioState != asIdle))
                  {
                     /* Check to see if we have started to receive SBC  */
                     /* packets and are in the process of bufferiing the*/
                     /* SBC packets.                                    */
                     if(PlaybackContext.AudioState == asBuffering)
                     {
                        /* Buffer SBC packets until the SBC packet      */
                        /* buffer is 1/4 full (small jitter).           */
                        if(PlaybackContext.SBCUsed >= (PlaybackContext.SBCEnd >> 2))
                           PlaybackContext.AudioState = asDecoding;
                     }

                     /* Check to see if we are decoding or playing.     */
                     if(PlaybackContext.AudioState != asBuffering)
                     {
                        /* While there are SBC packets in the buffer    */
                        /* then we will attempt to decode the packet.   */
                        while(PlaybackContext.SBCUsed)
                        {
                           /* If there is not enough space left to      */
                           /* decode an SBC frame then we need to break */
                           /* out of the loop.                          */
                           if((AUDIO_BUFFER_SIZE - PlaybackContext.NumAudioSamples) < NUM_AUDIO_SAMPLES_PER_SBC_FRAME)
                              break;

                           /* Determine the amount of data that we can  */
                           /* process.                                  */
                           if(PlaybackContext.SBCIn > PlaybackContext.SBCOut)
                              BytesUsed = (PlaybackContext.SBCIn - PlaybackContext.SBCOut);
                           else
                              BytesUsed = (PlaybackContext.SBCEnd - PlaybackContext.SBCOut);

                           /* Release the lock.                         */
                           BSC_ReleaseListLock();

                           /* Attempt to decode an SBC frame.           */
                           BytesUsed = Decode(BytesUsed, &(PlaybackContext.SBCBuffer[PlaybackContext.SBCOut]));

                           /* Re-acquire the lock because we need to    */
                           /* operate on some variables that need to be */
                           /* protected.                                */
                           BSC_AcquireListLock();

                           if(BytesUsed > 0)
                           {
                              /* Adjust the index and free counts based */
                              /* on the number of bytes processed.      */
                              PlaybackContext.SBCOut  += BytesUsed;
                              PlaybackContext.SBCFree += BytesUsed;
                              PlaybackContext.SBCUsed -= BytesUsed;

                              /* Check to see if the buffer needs to be */
                              /* wrapped.                               */
                              if(PlaybackContext.SBCOut >= PlaybackContext.SBCEnd)
                                 PlaybackContext.SBCOut = 0;
                           }
                           else
                              break;
                        }
                     }
                  }

                  /* If we got here then it means we still hold the     */
                  /* Lock, so we need to go ahead and release the lock. */
                  BSC_ReleaseListLock();
               }
               else
               {
                  /* Faile BSC_AcquireListLock(), Try again in 5msec    */
                  BTPS_Delay(5);
               }

               /* Delay to allow time for either more data to arrive or */
               /* the audio buffer to play out.                         */
               /* * NOTE * Some OS's do not allow small enough          */
               /*          granularity when sleeping.  We will simply   */
               /*          yield time to other tasks and process        */
               /*          everything next time through the loop.       */
               BTPS_Delay(0);
            }
            else
            {
               BTPS_Delay(200);
            }
         }
         else
         {
            BTPS_Delay(200);
         }
      }
   }

   /* Make sure we signal that the thread is now shutting down.         */
   BTPS_SetEvent(PlaybackContext.ShutdownEvent);
   Display((" Exit Decode Thread !!!\r\n"));
   Display(("\r\nSink> \b"));
   return(NULL);
}

   /* The following function is the interrupt request handler for the   */
   /* I2S interface.                                                    */
void AUDIO_I2S_IRQ_HANDLER(void)
{
   /* Write the next word onto the transmit buffer.                     */
   if((PlaybackContext.NumAudioSamples) && (SPI_I2S_GetFlagStatus(AUDIO_I2S_BASE, SPI_I2S_FLAG_TXE)))
   {
      if(PlaybackContext.LeftChannelOut)
      {
         SPI_I2S_SendData(AUDIO_I2S_BASE, PlaybackContext.LeftChannelBuffer[PlaybackContext.OutIndex]);
         PlaybackContext.LeftChannelOut = FALSE;

         /* Increment the output index and handle any buffer wrap.      */
         PlaybackContext.NumAudioSamples--;
         PlaybackContext.OutIndex++;
         if(PlaybackContext.OutIndex >= PlaybackContext.EndIndex)
         {
            PlaybackContext.OutIndex = 0;
            PlaybackContext.EndIndex = AUDIO_BUFFER_SIZE;
         }
      }
      else
      {
         if(PlaybackContext.SampleRateAdjustment)
         {
            PlaybackContext.SampleRateAdjustmentCount ++;
            if(PlaybackContext.SampleRateAdjustmentCount > (PlaybackContext.CurrentSampleRate / PlaybackContext.SampleRateAdjustment))
            {
               PlaybackContext.SampleRateAdjustmentCount = 0;

               if(PlaybackContext.NumAudioSamples > 1)
               {
                  /* Skip the current sample.                           */
                  PlaybackContext.NumAudioSamples--;
                  PlaybackContext.OutIndex ++;
                  if(PlaybackContext.OutIndex >= PlaybackContext.EndIndex)
                  {
                     PlaybackContext.OutIndex = 0;
                     PlaybackContext.EndIndex = AUDIO_BUFFER_SIZE;
                  }
               }
            }
         }

         SPI_I2S_SendData(AUDIO_I2S_BASE, PlaybackContext.RightChannelBuffer[PlaybackContext.OutIndex]);
         PlaybackContext.LeftChannelOut = TRUE;
      }
   }

   /* If we run out of audio samples, disable the DAC interrupts and set*/
   /* the state to Waiting.  We will start playing again when we buffer */
   /* up enough audio samples.                                          */
   if(!PlaybackContext.NumAudioSamples)
   {
      SPI_I2S_ITConfig(AUDIO_I2S_BASE, SPI_I2S_IT_TXE, DISABLE);

      if(PlaybackContext.AudioState != asIdle)
         PlaybackContext.AudioState = asDecoding;
   }
}

   /* The following function initializes the audio decoder.  The first  */
   /* parameter is a valid Bluetooth Stack ID This function will return */
   /* zero on success and negative on error.                            */
int InitializeAudioDecoder(unsigned int BluetoothStackID, BD_ADDR_t ConnectedBD_ADDR)
{
   int Result = -1;
   BTPS_MemoryStatistics_t MemoryStatistics;

   /* Verify that we are not Initialized.                               */
   if(!Initialized)
   {
      BTPS_QueryMemoryUsage(&MemoryStatistics, TRUE);
      Display(("Before BTPS_MemInitialize for PlaybackContext heap used=%d, heap free size=%d\r\n", 
        MemoryStatistics.CurrentHeapUsed, MemoryStatistics.HeapSize - MemoryStatistics.CurrentHeapUsed));
       
      /* Initialize the Context structure information.                  */
      BTPS_MemInitialize(&PlaybackContext, 0, sizeof(PlaybackContext_t));

      /* Note the Bluetooth Stack ID.                                   */
      PlaybackContext.BluetoothStackID = BluetoothStackID;

      /* Flag the entire SBC buffer is empty.                           */
      PlaybackContext.SBCFree          = SBC_BUFFER_SIZE;

      /* Set the Audio State to Idle.                                   */
      PlaybackContext.AudioState        = asIdle;
      PlaybackContext.DisplayFormatInfo = 1;

      /* Retreive the current Stream Configuration.                     */
      Result = AUD_Query_Stream_Configuration(BluetoothStackID, ConnectedBD_ADDR, astSNK, &PlaybackContext.StreamConfig);
      if(!Result)
      {
         /* Note the current frequency                                  */
         PlaybackContext.CurrentSampleRate = PlaybackContext.StreamConfig.StreamFormat.SampleFrequency;

         if((PlaybackContext.StreamConfig.MediaCodecType == A2DP_MEDIA_CODEC_TYPE_SBC) &&
             (PlaybackContext.StreamConfig.MediaCodecInfoLength == sizeof(A2DP_SBC_Codec_Specific_Information_Element_t)))
         {
            if(((PlaybackContext.ShutdownEvent = BTPS_CreateEvent(FALSE)) != NULL) &&
                ((PlaybackContext.SBCDecodeEvent = BTPS_CreateEvent(FALSE)) != NULL))
            {
               /* Go ahead and configure the output device.             */
               if(  !Initialize_AUDIO(PlaybackContext.StreamConfig.StreamFormat.SampleFrequency))
               {
                  /* Now, we are ready to start decoding.  First, let's */
                  /* initialize the Decoder.                            */
                  if((PlaybackContext.DecoderHandle = SBC_Initialize_Decoder()) != NULL)
                  {
                     Initialized = TRUE;
                     /* Check if the PlaybackThread is already open != 0. Open it once! */
                     if(!DecoderThreadId)
                     {
                        if(DecoderThreadId = BTPS_CreateThread(PlaybackThread, DECODE_THREAD_STACK_SIZE, NULL))
                        {
                        Result = 0;
                            Display((" Start Playback Thread succedded with ID=%d\r\n", DecoderThreadId));
                            LastDecoderThreadId = DecoderThreadId;
                            BTPS_QueryMemoryUsage(&MemoryStatistics, TRUE);
                            Display(("After creating the Playback thread, heap used=%d, heap free size=%d\r\n", 
                                      MemoryStatistics.CurrentHeapUsed, MemoryStatistics.HeapSize - MemoryStatistics.CurrentHeapUsed));
                        }
                     else
                     {
                        Initialized = FALSE;
                            Display((" Error!!! Unable to start Playback Thread. DecoderThreadId=%d\r\n", DecoderThreadId));
                        }
                     }
                  }
                  else
                     Display((" Error!!! Failed to decoder.\r\n"));
               }
               else
                  Display((" Error!!! Failed to configure output device.\r\n"));
            }
            else
               Display((" Error!!! Failed to create mutex/event.\r\n"));
         }
         else
            Display((" Error!!! Unsupported stream type or invalid configuration\r\n"));
      }
   }
   else
      Display((" Error!!! Unable to query audio connection configuration (%d)\r\n", Result));

   return(Result);
}

   /* The following function is responsible for freeing all resources   */
   /* that were previously allocated for an audio decoder.              */
void CleanupAudioDecoder(void)
{
   BTPS_MemoryStatistics_t MemoryStatistics;
   Initialized = FALSE;

   /* Set the event to kill the thread                                  */
   if((PlaybackContext.ShutdownEvent) && (PlaybackContext.SBCDecodeEvent))
   {
      /* Flag that we would like the decode thread to exit.             */
      PlaybackContext.AudioState = asIdle ;
       
      PlaybackContext.Exit       = TRUE;

      BTPS_SetEvent(PlaybackContext.SBCDecodeEvent);

      /* PlaybackThread Thread allways on, go ahead and clean up all the */
      /* other resources.                                                */
      BTPS_CloseEvent(PlaybackContext.ShutdownEvent);
      PlaybackContext.ShutdownEvent = NULL;

      BTPS_CloseEvent(PlaybackContext.SBCDecodeEvent);
      PlaybackContext.SBCDecodeEvent = NULL;
   }

   if(PlaybackContext.DecoderHandle)
   {
      Display((" Clear SBC decoder\r\n"));
      /* We are all finished with the decoder, so we can inform the     */
      /* library that we are finished with the handle that we opened.   */
      SBC_Cleanup_Decoder(PlaybackContext.DecoderHandle);

      PlaybackContext.DecoderHandle = NULL;
   }

   PlaybackContext.AudioState = asIdle;

   if(PlaybackContext.LeftChannelBuffer)
      BTPS_FreeMemory(PlaybackContext.LeftChannelBuffer);

   (void) AUDIO_Pause_Resume_CODEC();
   
   BTPS_Delay(20);
   
   /* Un-initialize the Context structure information.                  */
   BTPS_FreeMemory(&PlaybackContext);
   
   BTPS_QueryMemoryUsage(&MemoryStatistics, TRUE);
   Display(("After BTPS_FreeMemory(&PlaybackContext), heap used=%d, heap free size=%d\r\n", 
             MemoryStatistics.CurrentHeapUsed, MemoryStatistics.HeapSize - MemoryStatistics.CurrentHeapUsed));
}

   /* The following function is used to process audio data.  The        */
   /* parameters to this function specify the raw encoded audio data    */
   /* length and data, respectively.  A negative value will be returned */
   /* on error.                                                         */
int ProcessAudioData(unsigned int DataLength, unsigned char *DataPtr)
{
   int          Result = -1;
   unsigned int SBCFrameLength;
   unsigned int NumSBCFrames;

   /* Ignore data if we are not initialized                             */
   if(Initialized)
   {
      /* Confirm we have a buffer, and the length.                      */
      if((DataPtr) && (DataLength))
      {
         NumSBCFrames = (DataPtr[0] & A2DP_SBC_HEADER_NUMBER_FRAMES_MASK);

         /* Determine the Number and Size of each SBC frames that are in*/
         /* the GAVD packet.                                            */
         SBCFrameLength = (DataLength / NumSBCFrames);
         if(PlaybackContext.AudioState == asIdle)
         {
            /* This is the first SBC frame received since the stream was*/
            /* started.  Initialize information about the data being    */
            /* received.                                                */
            PlaybackContext.SBCIn           = 0;
            PlaybackContext.SBCOut          = 0;
            PlaybackContext.SBCEnd          = SBC_BUFFER_SIZE;
            PlaybackContext.SBCFree         = PlaybackContext.SBCEnd;
            PlaybackContext.SBCUsed         = 0;
            PlaybackContext.SBCFrameLength  = SBCFrameLength;
            PlaybackContext.AudioState      = asBuffering;
            PlaybackContext.BufferLowLimit  = PlaybackContext.SBCEnd >> 1;
            PlaybackContext.BufferHighLimit = PlaybackContext.SBCEnd - (PlaybackContext.SBCEnd >> 2);

            Display(("Buffer High Limit: %d\r\n", PlaybackContext.BufferHighLimit));
            Display(("Buffer Low Limit : %d\r\n", PlaybackContext.BufferLowLimit));

            /* Go ahead and inform the decode thread that there is data */
            /* to process.                                              */
            if(PlaybackContext.SBCDecodeEvent)
               BTPS_SetEvent(PlaybackContext.SBCDecodeEvent);
         }

         /* The first byte of the data buffer contains the number of    */
         /* frames.  We need to account for this.                       */
         DataLength--;
         DataPtr++;

         /* Go ahead acquire a lock to protect the variables that are   */
         /* shared between this module and the playback thread.         */
         BSC_AcquireListLock();

         /* Move the SBC frames to the SBC buffer as long as there is   */
         /* room for the data.                                          */
         /* * NOTE * There are two choices here.  We can either:        */
         /*                                                             */
         /*             - drop the frames that we don't have room       */
         /*               for (i.e. these newest frames)                */
         /*             - drop the oldest frames to make room.          */
         /*                                                             */
         /*          Currently we are going to drop the newest          */
         /*          (incoming) frames.  Either way frames will have    */
         /*          to be dropped.                                     */
         if(PlaybackContext.SBCFree >= DataLength)
         {
            /* Verify that this copy will not go beyond the end the     */
            /* buffer.                                                  */
            if((PlaybackContext.SBCIn + DataLength) <= SBC_BUFFER_SIZE)
            {
               /* Single copy operation.                                */
               BTPS_MemCopy(&(PlaybackContext.SBCBuffer[PlaybackContext.SBCIn]), DataPtr, DataLength);
            }
            else
            {
               /* Multiple copy operation.                              */
               BTPS_MemCopy(&(PlaybackContext.SBCBuffer[PlaybackContext.SBCIn]), DataPtr, (SBC_BUFFER_SIZE - PlaybackContext.SBCIn));

               BTPS_MemCopy(&(PlaybackContext.SBCBuffer[0]), &(DataPtr[SBC_BUFFER_SIZE - PlaybackContext.SBCIn]), DataLength - (SBC_BUFFER_SIZE - PlaybackContext.SBCIn));
            }

            PlaybackContext.SBCFree -= DataLength;
            PlaybackContext.SBCIn   += DataLength;
            PlaybackContext.SBCUsed += DataLength;
            if(PlaybackContext.SBCIn >= SBC_BUFFER_SIZE)
            {
               /* Wrap the buffer pointer.                              */
               PlaybackContext.SBCIn -= SBC_BUFFER_SIZE;
            }
         }
         else
         {
            /* We need to truncate the amount of data we copy to into   */
            /* the buffer as there is not enough room for all the       */
            /* frames.                                                  */

            /* Truncation will occur on SBC frame boundaries.  this     */
            /* means that we will not insert partial SBC frames.        */

            while(NumSBCFrames)
            {
               /* See if the current SBC frame will fit into the buffer.*/
               if((SBCFrameLength = SBC_CalculateDecoderFrameSize(DataLength, DataPtr)) != 0)
               {
                  if(SBCFrameLength <= PlaybackContext.SBCFree)
                  {
                     /* Frame will fit, go ahead and copy it into the   */
                     /* buffer.                                         */

                     /* Verify that this copy will not go beyond the end*/
                     /* of the buffer.                                  */
                     if((PlaybackContext.SBCIn + SBCFrameLength) <= SBC_BUFFER_SIZE)
                     {
                        /* Single copy operation.                       */
                        BTPS_MemCopy(&(PlaybackContext.SBCBuffer[PlaybackContext.SBCIn]), DataPtr, SBCFrameLength);
                     }
                     else
                     {
                        /* Multiple copy operation.                     */
                        BTPS_MemCopy(&(PlaybackContext.SBCBuffer[PlaybackContext.SBCIn]), DataPtr, (SBC_BUFFER_SIZE - PlaybackContext.SBCIn));

                        BTPS_MemCopy(&(PlaybackContext.SBCBuffer[0]), &(DataPtr[(SBC_BUFFER_SIZE - PlaybackContext.SBCIn)]), SBCFrameLength - (SBC_BUFFER_SIZE - PlaybackContext.SBCIn));
                     }

                     PlaybackContext.SBCFree -= SBCFrameLength;
                     PlaybackContext.SBCIn   += SBCFrameLength;
                     PlaybackContext.SBCUsed += SBCFrameLength;
                     if(PlaybackContext.SBCIn >= SBC_BUFFER_SIZE)
                     {
                        /* Wrap the buffer pointer.                     */
                        PlaybackContext.SBCIn -= SBC_BUFFER_SIZE;
                     }

                     /* Move to the next SBC frame to process.          */
                     DataPtr += SBCFrameLength;

                     NumSBCFrames--;
                  }
                  else
                  {
                     Display(("Dropping %d SBC Frames\r\n", NumSBCFrames));
                     NumSBCFrames = 0;
                  }
               }
               else
                  NumSBCFrames = 0;
            }
         }

         /* Finished.  Go ahead and release the lock.                   */
         BSC_ReleaseListLock();

         /* Check to state of the SBC buffer to see if we need to adjust*/
         /* the playback speed.                                         */
         if((!PlaybackContext.SampleRateAdjustment) && (PlaybackContext.SBCUsed >= PlaybackContext.BufferHighLimit))
         {
            /* We have too many samples so we will increase the playback*/
            /* speed to free up storage.                                */
            PlaybackContext.SampleRateAdjustment = SAMPLE_RATE_ADJUSTMENT_VALUE;

            Display(("Up %d %d\r\n", (unsigned long)PlaybackContext.CurrentSampleRate, PlaybackContext.SBCUsed));
         }

         if((PlaybackContext.SampleRateAdjustment) && (PlaybackContext.SBCUsed <= PlaybackContext.BufferLowLimit))
         {
            /* We have too many samples so we will increase the playback*/
            /* speed to reduce the storage.                             */
            PlaybackContext.SampleRateAdjustment = 0;

            Display(("Down %d %d\r\n", (unsigned long)PlaybackContext.CurrentSampleRate, PlaybackContext.SBCUsed));
         }

         /* Flag that we were successful.                               */
         Result = 0;
      }
   }

   return(Result);
}

