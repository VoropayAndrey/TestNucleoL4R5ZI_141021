/*
 * serialInput.h
 *
 *  Created on: Jun 1, 2022
 *      Author: andrey
 */

#ifndef INC_SERIALINPUT_H_
#define INC_SERIALINPUT_H_

#include <stdint.h>

typedef void (*SerialInputPortInitialization(void));

void serialInputInit(SerialInputPortInitialization callback, uint32_t buffer_size);

// Call from Rx handler
void serialInputProcessInterrupt(uint8_t* buffer, uint32_t length);

// Call from RTOS task to read data
// Return number of received bytes
uint32_t serialInputRead(uint8_t* buffer);

#endif /* INC_SERIALINPUT_H_ */
