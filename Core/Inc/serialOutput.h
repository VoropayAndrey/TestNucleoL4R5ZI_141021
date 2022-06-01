/*
 * serialOutput.h
 *
 * Purpose of this layer is to combine different sources of output text messages from different threads
 * and tasks to one output, for example UART port in thread safe manner without blocking calling thread.
 *
 *  Created on: Jun 1, 2022
 *      Author: andrey
 */

#ifndef INC_SERIALOUTPUT_H_
#define INC_SERIALOUTPUT_H_

#include <stdint.h>

typedef void (*SerialOutputCallback)(uint8_t* message, uint32_t length);

void serialOutputInit(SerialOutputCallback callback);

// Call from multiple RTOS tasks
void serialOutputPrint(uint8_t* message, uint32_t length);

// Call in a RTOS task
void serialOutputProcess();


#endif /* INC_SERIALOUTPUT_H_ */
