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

#define NULL ((void*)0)

typedef void(*callback_function_t)(void) ;

typedef enum
{
	THREAD_INACTIVE = 1,
	THREAD_PENDING,
	THREAD_SUSPENDED
}Thread_State_t;
typedef struct TCB_s
{
	callback_function_t		callback_function;
	uint32_t 				stack_size;
	uint16_t				periodicity;
#if (KRTOS_SCHEDULER_TYPE == SCHD_TYPE_PREMEMPTIVE)
	uint8_t					priority;
#endif
} TCB_config_t;

typedef struct tcb
{
	int32_t *stackPt;
	uint16_t periodicity;
	Thread_State_t state;
#if (KRTOS_SCHEDULER_TYPE == SCHD_TYPE_PREMEMPTIVE)
	uint8_t  priority;
#endif
	struct tcb *nextPt;
}TCBType;

typedef enum osKernelReturn_e
{
	OSKERNEL_FAIL = 0,
	OSKERNEL_PASS =1
} osKernelReturn_t;

osKernelReturn_t osKernel_ThreadCreate( TCB_config_t* task);
osKernelReturn_t osKernel_PeriodicThreadCreate(TCB_config_t* task);
osKernelReturn_t osKernel_init(uint32_t quanta);
void osKernel_ThreadYield(void);
osKernelReturn_t osKernel_ThreadRemove( TCBType* task);
osKernelReturn_t osKernel_ThreadSuspend( TCBType* task);
osKernelReturn_t osKernel_backgroundTaskSet( callback_function_t function);


#endif /* INC_OSKERNEL_H_ */
