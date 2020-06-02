/*
 * BC127.h
 *
 *  Created on: Jul 28, 2019
 *      Author: Matt
 */

#ifndef BC127_H_
#define BC127_H_

#include <stdint.h>

enum bc127Command_e{
	BLANK,
	PLAY,
	PAUSE,
	PAIR,
	PAIR_MODE,
	RESET_BC127
};

typedef enum {
	INIT_STATE,
	A2DP_OPEN,
	A2DP_CLOSED,
	CHARGING,
	CHARGED_FULL,
	NOT_CHARGING,
	INQUIRY_OK,
	PAIR_OK,
	PAIR_ERROR
}bc127Notification_e;

extern bc127Notification_e Notification;

typedef enum {
	NOT_READY,
	READY,
	NOT_CONNECTED,
	CONNECTED,
	PLAYING,
	NOT_PLAYING
}bc127Status_e;

extern bc127Status_e Status;

typedef struct{
	char bluetoothAddress[12];
	uint8_t notification;
	uint8_t status;
} bc127Device_t;

void bc127SendCommand(enum bc127Command_e command);
void bc127Read(void);
void bc127UartHandler(void);

#define DATA 1
#define NO_DATA 0

#endif /* BC127_H_ */
