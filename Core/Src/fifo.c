/*
 * fifo.c
 *
 *  Created on: Jun 3, 2022
 *      Author: andrey
 */

#include "fifo.h"
#include <stdlib.h>


uint8_t fifoInit(FifoContext* context, uint32_t size, uint8_t type) {
	context->fifoBufferSize = size;
	context->fifoType = type;

	context->fifoHead = 0;
	context->fifoTail = 0;

	context->fifoMutex = xSemaphoreCreateMutexStatic(&(context->fifoMutexBuffer));

	context->fifoEmptySpaceLeft = context->fifoBufferSize;
	context->fifoBuffer = (uint8_t*)calloc(context->fifoBufferSize, sizeof(uint8_t));

	return context->fifoBuffer != NULL && context->fifoMutex != NULL;
}

uint8_t fifoPush(FifoContext* context, uint8_t data) {

	if(context->fifoType == FIFO_TYPE_ISR_PUSH) {
		xSemaphoreTakeFromISR(context->fifoMutex, pdFALSE);
	} else {
		xSemaphoreTake(context->fifoMutex, (TickType_t)portMAX_DELAY);
	}

	if(context->fifoEmptySpaceLeft > 0) {

		context->fifoBuffer[context->fifoHead] = data;

		if(context->fifoHead < context->fifoBufferSize - 1) {
			context->fifoHead++;
		} else {
			context->fifoHead = 0;
		}

		context->fifoEmptySpaceLeft--;
	} else {
		// Overflow!
		uint32_t error = 0;
		error++;

	}

	if(context->fifoType == FIFO_TYPE_ISR_PUSH) {
		xSemaphoreGiveFromISR(context->fifoMutex, pdFALSE);
	} else {
		xSemaphoreGive(context->fifoMutex);
	}

	return 1;
}

uint8_t fifoPull(FifoContext* context, uint8_t* data) {

	if(context->fifoType == FIFO_TYPE_ISR_PULL) {
		xSemaphoreTakeFromISR(context->fifoMutex, pdFALSE);
	} else {
		xSemaphoreTake(context->fifoMutex, (TickType_t)portMAX_DELAY);
	}

	if(context->fifoEmptySpaceLeft < context->fifoBufferSize) {
		*data = context->fifoBuffer[context->fifoTail];

		if(context->fifoTail < context->fifoBufferSize - 1) {
			context->fifoTail++;
		} else {
			context->fifoTail = 0;
		}
		context->fifoEmptySpaceLeft++;
	} else {
		// no data
		return 0;
	}

	if(context->fifoType == FIFO_TYPE_ISR_PULL) {
		xSemaphoreGiveFromISR(context->fifoMutex, pdFALSE);
	} else {
		xSemaphoreGive(context->fifoMutex);
	}

	return 1;
}
