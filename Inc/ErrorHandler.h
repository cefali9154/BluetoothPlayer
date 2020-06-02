/*
 * ErrorHandler.h
 *
 *  Created on: Jun 15, 2019
 *      Author: Matt
 */

#ifndef ERRORHANDLER_H_
#define ERRORHANDLER_H_

#include <stdint.h>
#include "globals.h"
#include <stdio.h>
#include "stm32l0xx_hal.h"

void Error_Handler(void);
void assert_failed(uint8_t *file, uint32_t line);

#endif /* ERRORHANDLER_H_ */
