# Car Black Box using PIC16F877A
## Overview
This project implements a Car Black Box System using the PIC16F877A microcontroller. The system continuously monitors and records important vehicle events such as gear changes, collisions, speed, and timestamps. Logged data is stored in external EEPROM memory and can later be viewed on the display or downloaded through UART.

The project demonstrates practical embedded systems concepts including ADC interfacing, I2C communication, RTC integration, EEPROM storage, UART communication, interrupt handling, and menu-driven user interfaces.

## Features
The system monitors and records important vehicle events in real time. It supports speed measurement using ADC, event logging with timestamps, collision detection, and gear change tracking.
The project also includes RTC integration using DS1307, external EEPROM storage, UART-based log downloading, password-protected access, and a menu-driven interface using a 16x2 CLCD.

## Hardware Components
- PIC16F877A Microcontroller
- 16x2 CLCD
- DS1307 RTC Module
- External EEPROM
- Digital Keypad
- ADC (Potentiometer 1) for speed simulation
- UART Terminal (Tera Term)

## Technologies Used
The project was developed using Embedded C in MPLAB X IDE with the XC8 compiler. 
It uses peripherals and protocols such as ADC, UART, I2C, timers, digital keypad and interrupts on the PIC16F877A microcontroller.

## Functional Modules
### Dashboard Display
The dashboard screen displays the current time, event, and speed on the CLCD.

### Event Logging
The system stores the timestamp, event type, and speed value for each event in external EEPROM memory.

### Login System
A password-protected login system is implemented with limited login attempts and temporary blocking after multiple incorrect entries.

### Log Management
Users can view stored logs, clear logs, and download logs through UART communication.

### RTC Management
The DS1307 RTC module is used to maintain real-time clock functionality and allows manual time setting.

## Communication Protocols Used
- I2C : Used for RTC and External EEPROM communications.
- UART : Used for downloading stored logs.

## Project Structure
```bash
.
├── adc.c
├── car_black_box_def.c
├── clcd.c
├── digital_keypad.c
├── ds1307.c
├── external_eeprom.c
├── i2c.c
├── isr.c
├── main.c
├── main.h
├── timers.c
├── uart.c
```
## Working Principle
The system continuously reads vehicle speed using ADC and monitors user inputs for events such as gear changes and collisions. Whenever an event occurs, the corresponding time, event type, and speed are stored in external EEPROM memory. Users can access the logs through a password-protected menu, view them on the CLCD, or download them through UART communication.

- The system returns to the default dashboard screen if idle for 5 seconds.
- Users are allowed a maximum of 3 password attempts.
- After multiple incorrect password attempts, the user is temporarily blocked.

## Key Functions
- SW1 -> Collision event
- SW2 -> Gear up
- SW3 -> Gear down
- SW4 -> Menu navigation / increment and represents binary '1'. Long press returns to the main menu screen
- SW5 -> Menu navigation / decrement and represents binary '0'. Long press returns to default screen and saves the updated time in Set Time mode

### Default Login Password
The default system password stored in EEPROM is 1010

### Logged Events
- ON -> System power ON
- C -> Collision detection
- GN -> Neutral gear
- GR -> Reverse gear
- G1-G4 -> Gear positions
