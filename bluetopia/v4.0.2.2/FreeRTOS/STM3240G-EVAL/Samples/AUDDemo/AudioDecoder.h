/*****< audiodecoder.h >*******************************************************/
/*      Copyright 2011 - 2013 Stonestreet One.                                */
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
/*   01/13/11  G. Hensley     Initial creation. (Based on LinuxDEVM)          */
/******************************************************************************/
#ifndef __AUDIODECODERH__
#define __AUDIODECODERH__

   /* The following function initializes the audio decoder.  The first  */
   /* parameter is a valid Bluetooth Stack ID This function will return */
   /* zero on success and negative on error.                            */
int InitializeAudioDecoder(unsigned int BluetoothStackID, BD_ADDR_t ConnectedBD_ADDR);

   /* The following function is responsible for freeing all resources   */
   /* that were previously allocated for an audio decoder.              */
void CleanupAudioDecoder(void);

   /* The following function is used to process audio data.  The        */
   /* parameters to this function specify the raw encoded audio data    */
   /* length and data, respectively.  A negative value will be returned */
   /* on error.                                                         */
int ProcessAudioData(unsigned int DataLength, unsigned char *DataPtr);

#endif

