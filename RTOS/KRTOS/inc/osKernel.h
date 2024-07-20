/*
 * osKernel.h
 *
 *  Created on: Jul 12, 2024
 *      Author: E0162112
 */

#ifndef INC_OSKERNEL_H_
#define INC_OSKERNEL_H_

#include "KRTOS_cfg.h"
#include "stm32f103x6.h"
#include <stdint.h>
#include "Time_Base.h"

typedef void(*callback_function_t)(void) ;

typedef struct TCB_s
{
	uint32_t* 				stack;
	struct TCB_s*			next_thread;
	callback_function_t		callback_function;
	uint32_t 				stack_size;
	uint32_t				periodicity;
	uint32_t				ex_time;
	int32_t*				stackpt;
	uint8_t					index;
	uint8_t					priority;
} TCB_t;

typedef enum osKernelReturn_e
{
	OSKERNEL_FAIL = 0,
	OSKERNEL_PASS =1
} osKernelReturn_t;

osKernelReturn_t osKernel_ThreadCreate();
osKernelReturn_t osKernel_init(uint32_t quanta);

#endif /* INC_OSKERNEL_H_ */
