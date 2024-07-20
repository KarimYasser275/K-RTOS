/*
 * Time_Base.h
 *
 *  Created on: Jul 9, 2024
 *      Author: E0162112
 */

#ifndef TIME_BASE_H_
#define TIME_BASE_H_

#include <stdint.h>

void timebase_init(void);
void SysTicK_incrementTick(void);
void Systick_Delay(uint32_t seconds);
void timebase_ReloadTimeChange(uint32_t time);


#endif /* TIME_BASE_H_ */
