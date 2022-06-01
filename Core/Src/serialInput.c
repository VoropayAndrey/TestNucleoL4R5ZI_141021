/*
 * serialInput.c
 *
 *  Created on: Jun 1, 2022
 *      Author: andrey
 */

#include "serialInput.h"
#include "FreeRTOS.h"
#include "semphr.h"

#define BUFFER_LENGTH		100

typedef struct {
	uint8_t buffer[2][BUFFER_LENGTH];
	uint8_t bufferReadPage;
	uint8_t bufferWritePage;
	uint32_t bufferIndexHead[2];
	uint32_t bufferIndexTail[2];
	uint8_t outputBuffer[BUFFER_LENGTH];
	SemaphoreHandle_t inputMutex;
	SemaphoreHandle_t pageMutex;
	StaticSemaphore_t inputMutexBuffer;
	StaticSemaphore_t pageMutexBxuffer;
} SerialInputContext;

static SerialInputContext context;

static void putChar(SerialInputContext* context, uint8_t byte) {
	uint8_t page = 0;
	if(xSemaphoreTakeFromISR(context->pageMutex, pdFALSE) == pdTRUE) {
		page = context->bufferWritePage;

		context->buffer[page][context->bufferIndexHead[page]] = byte;

		// bufferIndexHead should be less than 99 before incrementation
		if(context->bufferIndexHead[page] < BUFFER_LENGTH - 1) {
			context->bufferIndexHead[page]++;
		} else {
			// Full, what next?
			// Return to buffer start
			context->bufferIndexHead[page] = 0;
			// Or switch pages?
			// Because we need to write more data to the buffers
		}
		xSemaphoreGiveFromISR(context->pageMutex, pdFALSE);
	} else {

	}
}

static uint8_t pullChar(SerialInputContext* context, uint8_t page, uint8_t* data) {
	uint8_t result = 0;

	if(context->bufferIndexTail[page] == context->bufferIndexHead[page]) {
		// Buffer size is 0
	} else {
		// Buffer size is bigger than 0
		*data = context->buffer[page][context->bufferIndexTail[page]];
		result = 1;

		// Move to next byte
		if(context->bufferIndexTail[page] < BUFFER_LENGTH - 1) {
			context->bufferIndexTail[page]++;
		} else {
			// End is reached what next?
			// Return to start
			context->bufferIndexTail[page] = 0;
		}
	}
	return result;
}

void serialInputInit(SerialInputPortInitialization callback) {
	context.bufferReadPage = 0;
	context.bufferWritePage = 1;

	context.inputMutex = xSemaphoreCreateMutexStatic(&context.inputMutexBuffer);
	context.pageMutex = xSemaphoreCreateMutexStatic(&context.pageMutexBxuffer);

	xSemaphoreGive(context.inputMutex);
	xSemaphoreGive(context.pageMutex);
}

// Call from Rx handler
void serialInputProcessInterrupt(uint8_t* buffer, uint32_t length) {
	for(uint32_t i = 0; i < length; i++) {
		putChar(&context, buffer[i]);
	}
}

// Call from RTOS task to read data
// Return number of received bytes
uint32_t serialInputRead(uint8_t* buffer) {
	uint32_t dataLength = 0;
	uint8_t page = 0;
	if(xSemaphoreTake(context.pageMutex, pdFALSE) == pdTRUE) {
		page = context.bufferWritePage;

		if(context.bufferIndexHead[page] == context.bufferIndexTail[page]) {
			// Has no data
			dataLength = 0;
		} else if(context.bufferIndexHead[page] > context.bufferIndexTail[page]) {
			dataLength = context.bufferIndexHead[page] - context.bufferIndexTail[page];
		} else if(context.bufferIndexHead[page] < context.bufferIndexTail[page]) {
			dataLength = BUFFER_LENGTH - (context.bufferIndexTail[page] - context.bufferIndexHead[page]);
		}

		for(uint32_t i = 0; i < dataLength; i++) {
			pullChar(&context, page, &buffer[i]);
		}

		if(context.bufferReadPage) {
			context.bufferReadPage = 0;
			context.bufferWritePage = 1;
		} else {
			context.bufferReadPage = 1;
			context.bufferWritePage = 0;
		}


		xSemaphoreGive(context.pageMutex);
	}
	return dataLength;
}
