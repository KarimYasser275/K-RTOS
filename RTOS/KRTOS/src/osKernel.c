/*
 * osKernel.c
 *
 *  Created on: Jul 12, 2024
 *      Author: E0162112
 */


#include "osKernel.h"
#include "KRTOS_cfg.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define SYSTEM_CLK				8000000U
#define MILLIS_PRESCALER		1000U
#define TASKS_MAX_NUM			10U
#define PEND_SYSTICK_BIT		26U
#define PEND_SV_BIT				28U

static callback_function_t osKernel_background_task = NULL;

TCBType tcbs[TASKS_MAX_NUM] = {0};
TCBType* current_thread;

uint32_t periodic_counter = 0;

uint8_t Tasks_number = 0;
uint8_t PeriodicTasks_number = 0;
uint32_t TCB_Stack[NUM_OF_THREADS][STACK_SIZE];

uint32_t debug_scheduler_counter;
static void osKernelStack_Init(uint32_t index , int32_t stack_size);
static void osSchedular_Launch(void);
static void osKernel_Scheduler(void);
static void osKernel_Dispatcher(void);
static void osKernel_BackgroundTask(void);

TCB_config_t background_task = {.periodicity = 1001, .stack_size = 100, .callback_function = osKernel_BackgroundTask};


/**
 * Creates a new thread and adds it to the task control block (TCB).
 *
 * @param task Pointer to the task structure containing the details of the thread to be created.
 *
 * @return Returns OSKERNEL_PASS if the thread is successfully created and added to the TCB,
 *         and OSKERNEL_FAIL otherwise.
 *
 * @throws None
 */
osKernelReturn_t osKernel_ThreadCreate( TCB_config_t* task)
{
	osKernelReturn_t retval = OSKERNEL_FAIL;
	if (NULL != task)
	{
		__disable_irq();
		if (Tasks_number <= TASKS_MAX_NUM )
		{
			tcbs[Tasks_number].stackPt = (int32_t *)malloc(task->stack_size* sizeof(int32_t));
			/*Assign PC register to point to task address*/
			tcbs[Tasks_number].stackPt[task->stack_size - 2] = (int32_t)task->callback_function;
			tcbs[Tasks_number].periodicity = task->periodicity;

		#if (KRTOS_SCHEDULER_TYPE == SCHD_TYPE_PREMEMPTIVE)
			tcbs[Tasks_number].priority = task->priority;
		#endif
		
			osKernelStack_Init(Tasks_number, task->stack_size);
			retval = OSKERNEL_PASS;
			Tasks_number++;
		}
		__enable_irq();
		}
		return retval;
}


/**
 * Initializes the kernel stack for a given task.
 *
 * @param index The index of the task in the tcbs array.
 * @param stack_size The size of the stack in bytes.
 *
 * @return void
 *
 * @throws None
 */
static void osKernelStack_Init(uint32_t index , int32_t stack_size)
{
	/*PSR register , set T-bit (bit21) to 1to operate in thumb mode*/
	tcbs[index].stackPt[stack_size - 1] = 1U<<24;
	/*Initialize rest of registers by dummy value (optional for debugging)*/
	tcbs[index].stackPt[stack_size - 3] = 0xCCCCCCCC;  /*R14 (LR)*/
	tcbs[index].stackPt[stack_size - 4] = 0xDDDDDDDD;  /*R12*/
	tcbs[index].stackPt[stack_size - 5] = 0xEEEEEEEE;  /*R3*/
	tcbs[index].stackPt[stack_size - 6] = 0xFFFFFFFF;  /*R2*/
	tcbs[index].stackPt[stack_size - 7] = 0x11111111;  /*R1*/
	tcbs[index].stackPt[stack_size - 8] = 0x22222222;  /*R0*/

	tcbs[index].stackPt[stack_size - 9]  = 0x33333333;   /*R11*/
	tcbs[index].stackPt[stack_size - 10] = 0x44444444;  /*R10*/
	tcbs[index].stackPt[stack_size - 11] = 0x55555555;  /*R9*/
	tcbs[index].stackPt[stack_size - 12] = 0x66666666;  /*R8*/
	tcbs[index].stackPt[stack_size - 13] = 0x77777777;  /*R7*/
	tcbs[index].stackPt[stack_size - 14] = 0x88888888;  /*R6*/
	tcbs[index].stackPt[stack_size - 15] = 0x99999999;  /*R5*/
	tcbs[index].stackPt[stack_size - 16] = 0xBBBBBBBB;  /*R4*/

	/*Assign stack pointer to top of stack*/
	tcbs[index].stackPt += stack_size -16 ;
}


/**
 * Initializes the kernel by creating a circular linked list of task control blocks and configuring the SysTick timer.
 *
 * @param quanta The time period in milliseconds for the SysTick timer.
 *
 * @return The return value indicates whether the initialization was successful or not.
 *
 * @throws None
 */
osKernelReturn_t osKernel_init(uint32_t quanta)
{
	/*Create circular linked list to link task control blocks*/
	uint8_t i; osKernelReturn_t retval = OSKERNEL_PASS;
	osKernel_ThreadCreate(&background_task);
#if (KRTOS_SCHEDULER_TYPE == SCHD_TYPE_PREMEMPTIVE)
	/*Sort tasks by priority*/
	uint8_t j;
	for (i = 0; i < Tasks_number; i++)
	{
		for (j = i+1; j < Tasks_number; j++)
		{
			if (tcbs[i].priority < tcbs[j].priority)
			{
				TCBType temp = tcbs[i];
				tcbs[i] = tcbs[j];
				tcbs[j] = temp;
			}
		}
	}
#endif
	/* Create a circular linked list*/	
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
	/*Pass Scheduler to SysTick handler*/
	osSchedular_Launch();

	return retval;
}

/**
 * Launches the scheduler by restoring the state of the current thread from its task control block and returning from the exception.
 *
 * @return void
 *
 * @throws None
 */
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

/**
 * The SysTick_Handler function is responsible for handling the SysTick interrupt.
 * It suspends the current thread, saves the registers, loads the address of the current thread into r0,
 * loads the value of the current thread into r1, stores the Cortex-M SP at the address of r1,
 * chooses the next thread by loading the value from the location 4 bytes above r1,
 * stores the chosen thread into r0, loads the Cortex-M SP from the address of r1,
 * restores the registers, enables global interrupts, and returns from the exception.
 *
 * @return void
 *
 * @throws None
 */
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
	__asm("PUSH {R0,LR}");

	__asm("BL	osKernel_Scheduler");

	__asm("POP {R0,LR}");
	/*R1 =  currentPt i.e. New Thread*/
	 __asm("LDR		R1,[R0]");
	/*SP  = currentPt->StackPt*/
	 __asm("LDR		SP,[R1]");
	/*Restore r4,r5,r6,r7,r8,r9,r10,11*/
	__asm("POP {R4-R11}");

	/*Enable global interrupts*/
	__asm("CPSIE	I");

	/*Return from exception and restore r0,r1,r2,r3,r12,lr,pc,psr */
	__asm("BX	LR");


}

__attribute__((naked)) void PendSV_Handler(void)
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
	__asm("PUSH {R0,LR}");

	__asm("BL	osKernel_Dispatcher");

	__asm("POP {R0,LR}");
	/*R1 =  currentPt i.e. New Thread*/
	 __asm("LDR		R1,[R0]");
	/*SP  = currentPt->StackPt*/
	 __asm("LDR		SP,[R1]");
	/*Restore r4,r5,r6,r7,r8,r9,r10,11*/
	__asm("POP {R4-R11}");

	/*Enable global interrupts*/
	__asm("CPSIE	I");

	/*Return from exception and restore r0,r1,r2,r3,r12,lr,pc,psr */
	__asm("BX	LR");


}


/**
 * Triggers a Systick interrupt to switch to the next thread.
 *
 * @return void
 *
 * @throws None
 */
void osKernel_ThreadYield(void)
{
//	periodic_counter--; /*To counter act the periodic counter increment in scheduler as systick was fired manually*/
	/*Triggers Systick interrupt to switch to next thread*/
	/*Clear current Systick value*/
//	SysTick->VAL = 0;
	/*Pend Systick Interrupt*/
	SCB->ICSR |= 1 << PEND_SV_BIT;

}

/**
 * Removes a thread from the kernel.
 *
 * @param task a pointer to the thread to be removed
 *
 * @return the status of the removal operation
 *
 * @throws None
 */
osKernelReturn_t osKernel_ThreadRemove( TCBType* task)
{
	if (NULL == task) return OSKERNEL_FAIL;

	if (current_thread == task)
	{
		current_thread = task->nextPt;
		free(task->stackPt);
		return OSKERNEL_PASS;
	}

	TCBType* current = current_thread;
	while (current->nextPt != task)
	{
		current = current->nextPt;
		if (current == current_thread) return OSKERNEL_FAIL;
	}

	current->nextPt = task->nextPt;
	free(task->stackPt);
	return OSKERNEL_PASS;
}

osKernelReturn_t osKernel_ThreadSuspend ( TCBType* task)
{
	osKernelReturn_t retval = OSKERNEL_PASS;
	return retval;
}

static void osKernel_Dispatcher(void)
{
	for (uint8_t i =0; i < Tasks_number; i++)
	{
		current_thread = current_thread->nextPt;
		if(current_thread->state == THREAD_PENDING)
		{
			break;
		}
	}
	if (current_thread->state != THREAD_PENDING){
		current_thread = &tcbs[Tasks_number - 1];
	}
	current_thread->state = THREAD_INACTIVE;
}

static void __attribute__((used)) osKernel_Scheduler(void)
#if KRTOS_SCHEDULER_TYPE == SCHD_TYPE_PERIODIC
{
	periodic_counter++;

	for (uint8_t i =0; i < Tasks_number; i++)
	{
		if (!((periodic_counter) % tcbs[i].periodicity))
		{
			tcbs[i].state = THREAD_PENDING;
		}
	}
	osKernel_Dispatcher();
	if (periodic_counter == 1000)
	{
		periodic_counter = 0;
	}
}
#elif KRTOS_SCHEDULER_TYPE == SCHD_TYPE_ROUND_ROBIN
{
	current_thread = current_thread->nextPt; /* switch to next thread*/
}
#elif KRTOS_SCHEDULER_TYPE == SCHD_TYPE_PREMEMPTIVE
{
	periodic_counter++;

	for (uint8_t i =0; i < Tasks_number; i++)
	{
		if (!((periodic_counter) % tcbs[i].periodicity))
		{
			current_thread - &tcbs[i];
			break;
		}
	}
	if (periodic_counter == 1000)
	{
		periodic_counter = 0;
	}
}
#else 
#error "Invalid scheduler type"
#endif

static void osKernel_BackgroundTask(void)
{
	while(1){
		if(osKernel_background_task != NULL){
			osKernel_background_task();
		}
	}
}

osKernelReturn_t osKernel_backgroundTaskSet( callback_function_t function){
	osKernelReturn_t retval = OSKERNEL_FAIL;
	if(function != NULL){
		osKernel_background_task = function;
		retval = OSKERNEL_PASS;
	}
	return retval;
}
