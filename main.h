#ifndef MAIN_H
#define	MAIN_H

#include <xc.h>
#include <string.h>
#include "adc.h"
#include "clcd.h"
#include "digital_keypad.h"
#include "ds1307.h"
#include "i2c.h"
#include "car_black_box.h"
#include "external_eeprom.h"
#include "timers.h"
#include "uart.h"

#define DEFAULT_SCREEN          0x01
#define LOGIN_SCREEN            0x02
#define MAIN_MENU_SCREEN        0x03
#define VIEW_LOG_SCREEN         0x04
#define CLEAR_LOG_SCRREN        0x05
#define DOWNLOAD_LOG_SCREEN     0x06
#define SET_TIME_SCREEN         0x07
#define CHANGE_PASSWD_SCREEN    0x08

#define RESET_NOTHING           0x00
#define RESET_LOGIN_SCREEN      0x11
#define RESET_MENU              0x22
#define RESET_VIEW_LOG          0x33
#define RESET_LOG_MEM           0x44
#define RESET_TIME              0x55
#define RESET_PASSWD            0x66

#define LOGIN_SUCCESS           0x0F
#define RETURN_BACK_DEFAULT     0xF0
#define RETURN_BACK_MAIN_MENU   0xF1

#define PASS_ENTRY              0
#define PASS_RE_ENTRY           1

#endif	/* MAIN_H */

