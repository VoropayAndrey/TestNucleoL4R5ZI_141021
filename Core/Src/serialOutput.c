/*
 * serialOutput.c
 *
 *  Created on: Jun 1, 2022
 *      Author: andrey
 */


#include "serialOutput.h"
#include "fifo.h"
#include <stdlib.h>

#define BUFFER_LENGTH				8000
#define OVERFLOW_MESSAGE 			"OVF\n"
#define OVERFLOW_MESSAGE_LENGTH 	4

typedef struct {
	FifoContext fifoContext;
	uint8_t* outputBuffer;
	uint32_t bufferSize;
	SerialOutputProcessCallback outputCallbak;
} SerialOutputContext;

static SerialOutputContext context;

void serialOutputInit(SerialOutputProcessCallback output_callbak, SerialOutputOverflowCallback overflow_callback, uint32_t buffer_size, uint32_t overflow_size) {
	context.bufferSize = buffer_size;
	context.outputBuffer = (uint8_t*)calloc(buffer_size, sizeof(uint8_t));
	context.outputCallbak = output_callbak;
	fifoInit(&(context.fifoContext), buffer_size, FIFO_TYPE_NO_ISR);
}

void serialOutputPrint(uint8_t* message, uint32_t length) {
	for(uint32_t i = 0; i < length; i++) {
		fifoPush(&(context.fifoContext), message[i]);
	}
}

void serialOutputProcess() {
	uint32_t dataLength = context.fifoContext.fifoBufferSize - context.fifoContext.fifoEmptySpaceLeft;
	if(dataLength > 0) {
		for(uint32_t i = 0; i < dataLength; i++) {
			fifoPull(&(context.fifoContext), &(context.outputBuffer[i]));
		}
		context.outputCallbak(context.outputBuffer, dataLength);
	}

}
