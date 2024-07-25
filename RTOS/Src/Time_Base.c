#include "Time_Base.h"
#include "stm32f103x6.h"

#define ONE_SEC_LOAD											(8000000U)

#define SYSTICK_CTRL_ENABLE										((uint32_t)1U<<0)
#define SYSTICK_CTRL_TICK_INT									((uint32_t)1U<<1)
#define SYSTICK_CTRL_CLK_SRC									((uint32_t)1U<<2)
#define SYSTICK_CTRL_COUNT_FLAG									((uint32_t)1U<<16)
#define NULL ((void *)0)

callback_function_t SysTick_callbackFn = NULL;

uint32_t Systick_cnt = 0;

void timebase_init(void)
{
	/*Reload the timer with number of cycles per second*/
	SysTick->LOAD = ONE_SEC_LOAD -1;
	/*Clear Systick current value*/
	SysTick->VAL = 0;
	/*Select internal clock source*/
	SysTick->CTRL |=  SYSTICK_CTRL_CLK_SRC;
	/*Enable  tick interrupt*/
	SysTick->CTRL |=  SYSTICK_CTRL_TICK_INT;
	/*Enable count flag*/
	SysTick->CTRL |=  SYSTICK_CTRL_COUNT_FLAG;
	/*Enable SysTick*/
	SysTick->CTRL |=  SYSTICK_CTRL_ENABLE;
	/*Enable global interrupt*/
	__enable_irq();

}



void SysTicK_incrementTick(void)
{
	Systick_cnt++;
}


uint32_t SysTick_TickGet(void)
{
	uint32_t retval = 0;
	__disable_irq();
	retval = Systick_cnt;
	__enable_irq();
	return retval;
}

void Systick_Delay(uint32_t seconds)
{
	uint32_t current_systick_cnt = SysTick_TickGet();
	uint32_t destination = current_systick_cnt + seconds;
	while(SysTick_TickGet() <= destination);
}

void timebase_ReloadTimeChange(uint32_t time)
{
	if (time)
	{
		__disable_irq();
		/*disable systick*/
		SysTick->CTRL = 0;
		/*Clear Systick current value*/
		SysTick->VAL = 0;
		/*Reload the timer with number of cycles per second*/
		SysTick->LOAD = time -1;
		/*set SysTick to low priority*/
		NVIC_SetPriority(SysTick_IRQn,15);
		/*Enable SysTick*/
		SysTick->CTRL |= SYSTICK_CTRL_CLK_SRC;
		SysTick->CTRL |=  SYSTICK_CTRL_ENABLE;
		SysTick->CTRL |= SYSTICK_CTRL_TICK_INT;
		__enable_irq();
	}
}


