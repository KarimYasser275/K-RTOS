/*
 * osKernel.c
 *
 *  Created on: Jul 12, 2024
 *      Author: E0162112
 */


#include "osKernel.h"
#include <stdlib.h>

#define SYSTEM_CLK				8000000U
#define MILLIS_PRESCALER		1000U
#define TASKS_MAX_NUM			10U
TCBType tcbs[TASKS_MAX_NUM] = {0};
TCBType* current_thread;
uint8_t Tasks_number = 0;
uint32_t TCB_Stack[NUM_OF_THREADS][STACK_SIZE];

uint32_t debug_scheduler_counter;
static void osKernelStack_Init(uint32_t index , int32_t stack_size);
static void osSchedular_Launch(void);

osKernelReturn_t osKernel_ThreadCreate( TCB_t* task)
{
	osKernelReturn_t retval = OSKERNEL_FAIL;
	if (NULL != task)
	{
		__disable_irq();
		if (Tasks_number <= TASKS_MAX_NUM)
		{
			tcbs[Tasks_number].stackPt = (int32_t *)calloc(task->stack_size, sizeof(int32_t));
			/*Assign PC register to point to task address*/
			tcbs[Tasks_number].stackPt[task->stack_size * sizeof(int32_t) - 2] = task->callback_function;
	#if 0
			tcbs[Tasks_number].ex_time = task->ex_time;
			tcbs[Tasks_number].periodicity = task->periodicity;
			tcbs[Tasks_number].priority = task->priority;
			tcbs[Tasks_number].index = Tasks_number;
	#endif
			osKernelStack_Init(Tasks_number, task->stack_size);
			retval = OSKERNEL_PASS;
			Tasks_number++;
		}
		__enable_irq();
		}
}


static void osKernelStack_Init(uint32_t index , int32_t stack_size)
{
	stack_size *= sizeof(uint32_t);

	/*PSR register , set T-bit (bit21) to 1to operate in thumb mode*/
	tcbs[index].stackPt[stack_size - 1] = 1U<<24;
	/*Initialize rest of registers by dummy value (optional for debugging)*/
	tcbs[index].stackPt[stack_size - 3] = 0xAAAAAAAA;  /*R14 (LR)*/
	tcbs[index].stackPt[stack_size - 4] = 0xAAAAAAAA;  /*R12*/
	tcbs[index].stackPt[stack_size - 5] = 0xAAAAAAAA;  /*R3*/
	tcbs[index].stackPt[stack_size - 6] = 0xAAAAAAAA;  /*R2*/
	tcbs[index].stackPt[stack_size - 7] = 0xAAAAAAAA;  /*R1*/
	tcbs[index].stackPt[stack_size - 8] = 0xAAAAAAAA;  /*R0*/

	tcbs[index].stackPt[stack_size - 9]  = 0xAAAAAAAA;   /*R11*/
	tcbs[index].stackPt[stack_size - 10] = 0xAAAAAAAA;  /*R10*/
	tcbs[index].stackPt[stack_size - 11] = 0xAAAAAAAA;  /*R9*/
	tcbs[index].stackPt[stack_size - 12] = 0xAAAAAAAA;  /*R8*/
	tcbs[index].stackPt[stack_size - 13] = 0xAAAAAAAA;  /*R7*/
	tcbs[index].stackPt[stack_size - 14] = 0xAAAAAAAA;  /*R6*/
	tcbs[index].stackPt[stack_size - 15] = 0xAAAAAAAA;  /*R5*/
	tcbs[index].stackPt[stack_size - 16] = 0xAAAAAAAA;  /*R4*/

	/*Assign stack pointer to top of stack*/
	tcbs[index].stackPt += stack_size -16 ;
}


osKernelReturn_t osKernel_init(uint32_t quanta)
{
	/*Create circuler linked list to link task control blocks*/
	uint8_t i;
	for (i = 0; i<Tasks_number; i++)
	{
		if (i+1< Tasks_number)
		{
			tcbs[i].nextPt = &tcbs[i+1];
		}
		else
		{
			tcbs[i].nextPt = &tcbs[0];
		}
	}
	current_thread = &tcbs[0];
	/* configure SysTick to 1ms*/
	timebase_ReloadTimeChange(quanta * (SYSTEM_CLK/MILLIS_PRESCALER));
	/*set SysTick to low priority*/
	NVIC_SetPriority(SysTick_IRQn,15);
	/*Pass Scheduler to SysTick handler*/
	osSchedular_Launch();
}

static void osSchedular_Launch(void)
{
	/*Load address of current_thread into R0*/
	__asm("LDR R0,=current_thread");

	/*Load r2 from address equals r0,i.e r2 =current_thread*/
	__asm("LDR R2,[r0]");

	/*Load Cortex-M SP from address equals R2,i.e. SP = current_thread->stackPt*/
	__asm("LDR  SP,[R2]");

	/*Restore r4,r5,r6,r7,r8,r9,r10,r11*/
	__asm("POP {R4-R11}");

	/*Restore r12*/
	__asm("POP {R12}");

	/*Restore r0,r1,r2,r3*/
	__asm("POP	{R0-R3}");

	/*Skip LR */
	__asm("ADD  SP,SP,#4");

	/*Create a new start location by popping LR*/
	__asm("POP {LR}");

	/*Skip PSR by adding 4 to SP*/
	__asm("ADD  SP,SP,#4");

	/*Enable global interrupts*/
	__asm("CPSIE	I");

	/*Return from exception*/
	__asm("BX	LR");
}

__attribute__((naked)) void SysTick_Handler(void)
{
	/*SUSPEND CURRENT THREAD*/

	/*Disable global interrupts*/
	__asm("CPSID	I");

	/*Save r4,r5,r6,r7,r8,r9,r10,11*/
	__asm("PUSH  {R4-R11}");

	/*Load address of current_thread into r0*/
	__asm("LDR R0, =current_thread");

	/*Load r1 from address equals r0, i.e. r1 =current_thread*/
	__asm("LDR R1,[R0]");

	/*Store Cortex-M SP at address equals r1, i.e Save SP into tcb */
	__asm("STR SP,[R1]");

    /*CHOOSE NEXT THREAD*/

    /*Load r1 from a location 4bytes above address r1, i.e r1 = current_thread->next*/
	__asm("LDR R1,[R1,#4]");

	/*Store r1 at address equals r0, i.e current_thread =  r1*/
	__asm("STR	R1,[R0]");

	/*Load Cortex-M SP from address equals r1, i.e SP =  current_thread->stackPt*/
	__asm("LDR SP,[R1]");

	/*Restore r4,r5,r6,r7,r8,r9,r10,11*/
	__asm("POP {R4-R11}");

	/*Enable global interrupts*/
	__asm("CPSIE	I");

	/*Return from exception and restore r0,r1,r2,r3,r12,lr,pc,psr */
	__asm("BX	LR");



}

