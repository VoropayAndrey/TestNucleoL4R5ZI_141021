/*****< audio.h >*************************************************************/
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
#ifndef AUDIO_H_
#define AUDIO_H_

#define AUDIO_ERROR_INVALID_PARAMETER     (-3000)
#define AUDIO_ERROR_I2C_OPERATION_FAILED  (-3001)

   /* The following function is used to initailize the CODEC interface. */
   /* It accepts as its parameter the sample Frequency to initailize to.*/
   /* This function returns zero if successful or a negative value if   */
   /* there was an error.                                               */
int Initialize_AUDIO(unsigned long Frequency);

   /* The following function un-initilizes the codec and disables       */
   /* playback and recording.  This function will return zero if        */
   /* successful or a negative value if there was an error.             */
int Un_Initialize_AUDIO(void);

   /* The following function Mutes/Un-Mutes the HW CODEC and puts       */
   /* the CODEC in Power Save mode. This function will return zero if   */
   /* successful or a negative value if there was an error.             */
int AUDIO_Pause_Resume_CODEC(void);

   /* The following function sets the playback frequency for the CODEC  */
   /* interface.  This function returns zero if successful or a negative*/
   /* value if there was an error.                                      */
int AUDIO_Set_Frequency(unsigned long Frequency);

   /* The following function will set the volume of the audio output.   */
   /* It accepts as its parameter the new volume as a percentage between*/
   /* zero (off) and 100.  This function return zero if successful or a */
   /* negative value if there was an error.                             */
int AUDIO_Set_Volume(unsigned int NewVolume);

   /* The following function returns the current level of the audio     */
   /* volume as a percentage between zero (muted) and 100.  This        */
   /* function returns the volume if successful or a negative value if  */
   /* there was an error.                                               */
int AUDIO_Get_Volume(void);

#endif
