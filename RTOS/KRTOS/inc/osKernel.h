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
	int32_t* 				stack;
	struct TCB_s*			next_thread;
	callback_function_t		callback_function;
	uint32_t 				stack_size;
	uint32_t				periodicity;
	uint32_t				ex_time;
	int32_t*				stackpt;
	uint8_t					index;
	uint8_t					priority;
} TCB_t;

typedef struct tcb
{
	int32_t *stackPt;
	struct tcb *nextPt;
}TCBType;

typedef struct TCB_peridoic_s
{
	callback_function_t		functionPt;
	uint32_t 				periodicity;
}TCB_peridoic_t;

typedef enum osKernelReturn_e
{
	OSKERNEL_FAIL = 0,
	OSKERNEL_PASS =1
} osKernelReturn_t;

osKernelReturn_t osKernel_ThreadCreate( TCB_t* task);
osKernelReturn_t osKernel_PeriodicThreadCreate(TCB_t* task);
osKernelReturn_t osKernel_init(uint32_t quanta);
void osKernel_ThreadYield(void);
osKernelReturn_t osKernel_ThreadRemove( TCBType* task);
osKernelReturn_t osKernel_ThreadSuspend( TCBType* task);

#endif /* INC_OSKERNEL_H_ */
