/*
 * serialInput.c
 *
 *  Created on: Jun 1, 2022
 *      Author: andrey
 */

#include "serialInput.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "fifo.h"

typedef struct {
	FifoContext fifoContext;
	uint32_t bufferSize;
} SerialInputContext;

static SerialInputContext context;

void serialInputInit(SerialInputPortInitialization callback, uint32_t buffer_size) {
	context.bufferSize = buffer_size;
	fifoInit(&(context.fifoContext), buffer_size, FIFO_TYPE_ISR_PUSH);
}

// Call from Rx handler
void serialInputProcessInterrupt(uint8_t* buffer, uint32_t length) {
	for(uint32_t i = 0; i < length; i++) {
		fifoPush(&(context.fifoContext), buffer[i]);
	}
}

// Call from RTOS task to read data
// Return number of received bytes
uint32_t serialInputRead(uint8_t* buffer) {
	uint32_t dataLength = context.fifoContext.fifoBufferSize - context.fifoContext.fifoEmptySpaceLeft;
	for(uint32_t i = 0; i < dataLength; i++) {
		fifoPull(&(context.fifoContext), buffer);
	}

	return dataLength;
}
