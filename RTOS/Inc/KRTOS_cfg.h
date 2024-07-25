/*
 * KTROS_cfg.h
 *
 *  Created on: Jul 12, 2024
 *      Author: E0162112
 */

#ifndef KRTOS_CFG_H_
#define KRTOS_CFG_H_

#include "osKernel.h"

typedef enum KRTOS_schd_e
{
	SCHD_TYPE_ROUND_ROBIN,
	SCHD_TYPE_COOPRATIVE,
	SCHD_TYPE_PERIODIC
}KRTOS_Schd_t;


#define NUM_OF_THREADS				3
#define STACK_SIZE					100 /* 100 word = 400 byte*/

#define KRTOS_SCHEDULER_TYPE		ROUND_ROBIN

#endif /* KRTOS_CFG_H_ */
