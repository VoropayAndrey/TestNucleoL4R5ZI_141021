/*
 * fifo.h
 *
 *  Created on: Jun 3, 2022
 *      Author: andrey
 */

#ifndef INC_FIFO_H_
#define INC_FIFO_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

#define FIFO_TYPE_ISR_PUSH		0
#define FIFO_TYPE_ISR_PULL		1
#define FIFO_TYPE_NO_ISR		2

typedef struct {
	uint32_t fifoBufferSize;
	uint32_t fifoEmptySpaceLeft;
	uint32_t fifoTail;
	uint32_t fifoHead;
	uint8_t* fifoBuffer;
	SemaphoreHandle_t fifoMutex;
	StaticSemaphore_t fifoMutexBuffer;
	uint8_t fifoType;

} FifoContext;

uint8_t fifoInit(FifoContext* context, uint32_t size, uint8_t fifoType);
uint8_t fifoPush(FifoContext* context, uint8_t data);
uint8_t fifoPull(FifoContext* context, uint8_t* data);

#endif /* INC_FIFO_H_ */
