/*
 * fifoTests.c
 *
 *  Created on: Jun 3, 2022
 *      Author: andrey
 */

#include "tests/fifoTests.h"
#include "fifo.h"
#include "_ansi.h"

void runFifoTest1() {
	char* testString = "0123456789";
	char expectedString[10];

	FifoContext context;
	fifoInit(&context, 10, FIFO_TYPE_NO_ISR);

	for(uint32_t i = 0; i < 5; i++) {
		fifoPush(&context, (uint8_t)testString[i]);
	}

	for(uint32_t i = 5; i < 7; i++) {
		fifoPush(&context, (uint8_t)testString[i]);
	}

	for(uint32_t i = 0; i < 2; i++) {
		fifoPull(&context, (uint8_t*)(&expectedString[i]));
	}

	for(uint32_t i = 7; i < 10; i++) {
		fifoPush(&context, (uint8_t)testString[i]);
	}

	for(uint32_t i = 2; i < 10; i++) {
		fifoPull(&context, (uint8_t*)(&expectedString[i]));
	}

	for(uint32_t i = 0; i < 10; i++) {
		//assert(testString[i] == expectedString[i]);
	}

}


