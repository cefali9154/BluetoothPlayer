/*
 * globals.h
 *
 *  Created on: Jul 13, 2019
 *      Author: Matt
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

//#define AUDIO_BUFFER_SIZE 512
//includes
#include "stm32l0xx_hal.h"
#include "stm32l0xx_ll_usart.h"
#include <stdio.h>
#include <string.h>
#include "ffconf.h"
#include "Settings.h"
#include <errno.h>
#include <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO


extern I2S_HandleTypeDef hi2s2;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern DMA_HandleTypeDef huart2_rx;

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;

extern SPI_HandleTypeDef hspi2;
extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi2_tx;

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

extern uint8_t usart1data;
extern uint8_t usart2data;

extern uint8_t bc127DataBuff[100];
extern uint8_t bc127DataBuffCt;
extern uint8_t waitingForUART;
extern uint16_t dataBufferCT;


//prototypes
int _write(int file, char *data, int len);
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);
#endif /* GLOBALS_H_ */
