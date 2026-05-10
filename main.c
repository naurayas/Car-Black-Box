/*
 * Name         : Naura Yasmeen U
 * Start Date   : 3 March, 2026
 * End Date     : 14 March, 2026
 * Project Name : CAR BLACK BOX
 */

#include "main.h"

// turn off WDT
#pragma config WDTE = OFF

// initially no switch pressed
unsigned char sw_p = NO_KEY;

static void init_config(void) {
    // init uart
    init_uart(9600);
    // init i2c
    init_i2c(100000);
    // init rtc
    init_ds1307();
    // init dkp
    init_digital_keypad();
    // init adc
    init_adc();
    // init clcd
    init_clcd();
    // init timer2
    init_timer2();
    
    // enable interrupts
    PEIE = 1;
    GIE = 1;
}

void main(void) {
    
    unsigned char operation_flag = DEFAULT_SCREEN, reset_flag = RESET_NOTHING ; // default
    // car powered on
    char event[3] = "ON";
    unsigned char speed = 0;
    // return values for operations
    unsigned char menu, view, time, passwd;
    // array for gears
    char *gear[] = {"GN", "GR", "G1", "G2", "G3", "G4"};
    // gear tracker
    unsigned char gr = 0;
    
    unsigned char key;
    unsigned char prev_key = ALL_RELEASED;
    // press time counter
    unsigned int press_time = 0;
    
    // initialize peripherals
    init_config();
    
    // log first event
    log_event(event, speed);
    
    // store correct password
    str_write_ext_eeprom(0x00, "1010");
    
    while (1) {
        
        // read speed from potentiometer 1
        speed = (unsigned char)(read_adc() / 10);     // 0 - 1023
        // speed : 0 - 99
        if (speed >= 100)
        {
            speed = 99;
        }
        
        // read key
        if (operation_flag == DEFAULT_SCREEN || operation_flag == LOGIN_SCREEN || operation_flag == CHANGE_PASSWD_SCREEN)
        {
            key = read_digital_keypad(STATE);
            for (unsigned int i = 300; i--; );
        }
        else if (operation_flag == MAIN_MENU_SCREEN || operation_flag == VIEW_LOG_SCREEN || operation_flag == SET_TIME_SCREEN)
        {
            key = read_digital_keypad(LEVEL);
            for (unsigned int i = 20000; i--; );
        }
        
        // store event and log collision
        if (key == SW1)
        {
            strcpy(event, "C ");
            log_event(event, speed);;
        }
        // store event and log gear change (up)
        else if (key == SW2 && gr < 6)
        {
            strcpy(event, gear[gr]);
            gr++;
            log_event(event, speed);
        }
        // store event and log gear change (down)
        else if (key == SW3 && gr > 0)
        {
            gr--;
            strcpy(event, gear[gr]);
            log_event(event, speed);
        }
        // go to login screen if sw4/sw5 pressed in default screen
        else if (operation_flag == DEFAULT_SCREEN && (key == SW4 || key == SW5))
        {
            operation_flag = LOGIN_SCREEN;      
            reset_flag = RESET_LOGIN_SCREEN;
            clear_screen();
            // display message and turn on cursor
            clcd_print("ENTER PASSWORD", LINE1(1));
            clcd_write(LINE2(4), INST_MODE);
            clcd_write(DISP_ON_CURSOR_ON, INST_MODE);
            // turn on timer2
            TMR2ON = 1;
        }
        // sw4 long press while in main menu screen to select menu
        else if (operation_flag == MAIN_MENU_SCREEN && sw_p == LP && key == SW4)
        {
            switch(menu)
            {
                case 0:
                    // view log
                    operation_flag = VIEW_LOG_SCREEN;
                    reset_flag = RESET_VIEW_LOG;
                    break;
                case 1:
                    // clear log
                    operation_flag = CLEAR_LOG_SCRREN;
                    reset_flag = RESET_LOG_MEM;
                    break;
                case 2:
                    // download log
                    operation_flag = DOWNLOAD_LOG_SCREEN;
                    break;
                case 3:
                    // set time
                    operation_flag = SET_TIME_SCREEN;
                    reset_flag = RESET_TIME;
                    break;
                case 4:
                    // change passwd
                    operation_flag = CHANGE_PASSWD_SCREEN;
                    reset_flag = RESET_PASSWD;
                    break;
            }
            clear_screen();
            press_time = 0;
            key = prev_key = ALL_RELEASED;
            sw_p = NO_KEY;
            // turn off timer2
            TMR2ON = 0;
        }
        // sw5 long press to go back to default screen while in main menu screen
        else if (operation_flag == MAIN_MENU_SCREEN && sw_p == LP && key == SW5)
        {
            operation_flag = DEFAULT_SCREEN;
            clear_screen();
            press_time = 0;
            key = prev_key = ALL_RELEASED;
            sw_p = NO_KEY;
            // turn off timer2
            TMR2ON = 0;
        }
        
        switch (operation_flag)
        {
            case DEFAULT_SCREEN: 
                display_dashboard(event, speed);
                break;
            case LOGIN_SCREEN:
                switch(login(key, reset_flag))
                {
                    case RETURN_BACK_DEFAULT:
                        clear_screen();
                        // change to default screen when idle for 5s
                        operation_flag = DEFAULT_SCREEN;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        TMR2ON = 0;
                        break;
                    case LOGIN_SUCCESS:
                        clear_screen();
                        // change to main menu screen for correct password
                        operation_flag = MAIN_MENU_SCREEN;
                        reset_flag = RESET_MENU;
                        continue;
                }
                break;
            case MAIN_MENU_SCREEN:
                if (key == SW4 || key == SW5)
                {
                    prev_key = key;
                    if (++press_time >= 10)
                        sw_p = LP;
                }
                else if (press_time > 0 && press_time < 10)
                {
                    sw_p = SP;
                    key = prev_key;
                    prev_key = ALL_RELEASED;
                    press_time = 0;
                }
                else
                {
                    press_time = 0;
                }
                
                // display menu screen
                if (sw_p == SP)
                {
                    menu = menu_screen(key, reset_flag);
                    sw_p = NO_KEY;
                }
                else if (sw_p == NO_KEY)
                {
                    key = ALL_RELEASED;
                    menu = menu_screen(key, reset_flag);
                }
                
                if (menu == RETURN_BACK_DEFAULT)
                {
                    clear_screen();
                    // change to default screen when idle for 5s
                    operation_flag = DEFAULT_SCREEN;
                    // turn off timer2
                    TMR2ON = 0;
                }
                break;
            case VIEW_LOG_SCREEN:
                if (key == SW4 || key == SW5)
                {
                    prev_key = key;
                    if (++press_time >= 10)
                        sw_p = LP;
                }
                else if (press_time > 0 && press_time < 10)
                {
                    sw_p = SP;
                    key = prev_key;
                    prev_key = ALL_RELEASED;
                    press_time = 0;
                }
                else
                {
                    press_time = 0;
                }
                
                // display logs
                if (sw_p == SP)
                {
                    view = view_log(key, reset_flag);
                    sw_p = NO_KEY;
                }
                else if (sw_p == NO_KEY)
                {
                    key = ALL_RELEASED;
                    view = view_log(key, reset_flag);
                }
                else if (sw_p == LP)
                {
                    key = prev_key;
                    prev_key = ALL_RELEASED;
                    view = view_log(key, reset_flag);
                    sw_p = NO_KEY;
                }
                
                switch (view)
                {
                    case RETURN_BACK_MAIN_MENU:
                        view = 0;
                        clear_screen();
                        // change to main menu screen for sw4 long press
                        operation_flag = MAIN_MENU_SCREEN;
                        press_time = 0;
                        reset_flag = RESET_MENU;
                        // turn on  timer2
                        TMR2ON = 1;
                        continue;
                    case RETURN_BACK_DEFAULT:
                        view = 0;
                        clear_screen();
                        // change to default screen for sw5 long press
                        operation_flag = DEFAULT_SCREEN;
                        press_time = 0;
                        break;
                }
                break;
            case CLEAR_LOG_SCRREN:
                clear_log(reset_flag);
                clear_screen();
                // change to default screen after clearing logs
                operation_flag = DEFAULT_SCREEN;
                break;
            case DOWNLOAD_LOG_SCREEN:
                download_log();
                clear_screen();
                // change to default screen after downloading logs
                operation_flag = DEFAULT_SCREEN;
                break;
            case SET_TIME_SCREEN:
                if (key == SW4 || key == SW5)
                {
                    prev_key = key;
                    if (++press_time > 10)
                        sw_p = LP;
                }
                else if (press_time > 0 && press_time < 10)
                {
                    sw_p = SP;
                    key = prev_key;
                    prev_key = ALL_RELEASED;
                    press_time = 0;
                }
                else
                {
                    press_time = 0;
                }
                
                // display set time screen
                if (sw_p == SP)
                {
                    time = set_time(key, reset_flag);
                    sw_p = NO_KEY;
                }
                else if (sw_p == NO_KEY)
                {
                    key = ALL_RELEASED;
                    time = set_time(key, reset_flag);
                }
                else if (sw_p == LP)
                {
                    key = prev_key;
                    prev_key = ALL_RELEASED;
                    time = set_time(key, reset_flag);
                    sw_p = NO_KEY;
                }
                
                switch (time)
                {
                    case RETURN_BACK_MAIN_MENU:
                        time = 0;
                        clear_screen();
                        // change to main menu screen for sw4 long press
                        operation_flag = MAIN_MENU_SCREEN;
                        press_time = 0;
                        reset_flag = RESET_MENU;
                        TMR2ON = 1;
                        continue; 
                    case RETURN_BACK_DEFAULT:
                        time = 0;
                        clear_screen();
                        // change to default screen after saving time - sw5 long press
                        operation_flag = DEFAULT_SCREEN;
                        break;
                }
                break;
            case CHANGE_PASSWD_SCREEN:
                passwd = change_passwd(key, reset_flag);
                TMR2ON = 1;
                // change to default screen when idle for 5s
                if (passwd == RETURN_BACK_DEFAULT)
                {
                    passwd = 0;
                    clear_screen();
                    operation_flag = DEFAULT_SCREEN;
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    TMR2ON = 0;
                }
                // change to main menu screen after password re-entry
                else if (passwd == RETURN_BACK_MAIN_MENU)
                {
                    passwd = 0;
                    clear_screen();
                    operation_flag = MAIN_MENU_SCREEN;
                    reset_flag = RESET_MENU;
                    TMR2ON = 1;
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    continue;
                }
                break;
        }
        reset_flag = RESET_NOTHING;
    }
}
