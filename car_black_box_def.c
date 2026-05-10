// function definitions

#include "main.h"


unsigned char clock_reg[3];
char time[7];   // hhmmss
char log[11];     //hhmmssevsp
// log position tracker
char pos = 0;
unsigned char log_flag = 0, log_count = 0;
char sec;
// idle state tracker
char return_time;
// store menu items
char *menu[] = {"View log", "Clear log", "Download log", "Set time", "Change passwd"};
static char menu_pos;
// store time, event, speed to get saved logs
char saved_time[7], saved_ev[3], saved_sp[3];
extern unsigned char sw_p;


static void get_time(void)
{
    clock_reg[0] = read_ds1307(HOUR_ADDR); // HH -> BCD 
    clock_reg[1] = read_ds1307(MIN_ADDR); // MM -> BCD 
    clock_reg[2] = read_ds1307(SEC_ADDR); // SS -> BCD 
    
    // BCD to ascii
    // HH -> 
    time[0] = ((clock_reg[0] >> 4) & 0x03) + '0';
    time[1] = (clock_reg[0] & 0x0F) + '0';
    
    // MM 
    time[2] = ((clock_reg[1] >> 4) & 0x07) + '0';
    time[3] = (clock_reg[1] & 0x0F) + '0';
    
    // SS
    time[4] = ((clock_reg[2] >> 4) & 0x07) + '0';
    time[5] = (clock_reg[2] & 0x0F) + '0';
    time[6] = '\0';
}

static void display_time(void)
{
    get_time();
    clcd_putch(time[0], LINE2(1));
    clcd_putch(time[1], LINE2(2));
    clcd_putch(':', LINE2(3));
    clcd_putch(time[2], LINE2(4));
    clcd_putch(time[3], LINE2(5));
    clcd_putch(':', LINE2(6));
    clcd_putch(time[4], LINE2(7));
    clcd_putch(time[5], LINE2(8));
}

void display_dashboard(char event[], unsigned char speed)
{
    clcd_print("TIME     EV  SP", LINE1(1));
    
    // display time
    display_time();
    
    // display event on clcd
    clcd_print(event, LINE2(10));
    
    // display speed on clcd
    clcd_putch(speed / 10 + '0', LINE2(14));
    clcd_putch(speed % 10 + '0', LINE2(15));
}

static void log_car_event(void)
{
    char addr = 0x05;
    if (pos == 10)
    {
        pos = 0;
    }
    
    addr = pos * 10 + addr;      // 0+5=5, 10+5=15, 20+5=25....
    str_write_ext_eeprom(addr, log);
    pos++;
    
    // track count of logs
    if (log_count < 9)
        log_count++;
}

void log_event(char event[], unsigned char speed)
{
    // set log_flag for at-least one event occurred
    log_flag = 1;
    get_time();
    // store time
    strncpy(log, time, 6);  //hhmmss
    // store event
    strncpy(&log[6], event, 2);
    // store speed
    log[8] = speed / 10 + '0';
    log[9] = speed % 10 + '0';
    log[10] = '\0';
    
    // save to eeprom
    log_car_event();
}

void clear_screen(void)
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}

unsigned char login(unsigned char key, unsigned char reset_flag)
{
    static char user_passwd[4];
    static unsigned char i;    // 4 digit password
    static unsigned char attempts_left;     // max 3 attempts
    
    if (reset_flag == RESET_LOGIN_SCREEN)
    {
        i = 0;
        attempts_left = 3;
        user_passwd[0] = '\0';
        user_passwd[1] = '\0';
        user_passwd[2] = '\0';
        user_passwd[3] = '\0';
        key = ALL_RELEASED;
        return_time = 5;
    }
    
    // return if idle for 5s
    if (return_time == 0)
        return RETURN_BACK_DEFAULT;
    
    // store user entered password
    if (key == SW4 && i < 4)     // sw4 - '1'
    {
        user_passwd[i++] = '1';
        clcd_putch('*', LINE2(4 + i));
        return_time = 5;
    }
    else if (key == SW5 && i < 4)    // sw5 - '0'
    {
        user_passwd[i++] = '0';
        clcd_putch('*', LINE2(4 + i));
        return_time = 5;
    }
    
    // 4 digits entered
    if (i == 4)
    {
        // get the original password
        char stored_passwd[4];
        for (int j = 0; j < 4; j++)
        {
            stored_passwd[j] = read_external_eeprom(j);
        }
        
        // compare with original password
        if (!strncmp(user_passwd, stored_passwd, 4))
        {
            clear_screen();
            clcd_print("Login Success", LINE1(0));
            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
            __delay_ms(3000);
            return LOGIN_SUCCESS;
        }
        else
        {
            attempts_left--;
            if (attempts_left == 0)
            {
                clear_screen();
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                clcd_print("You are blocked", LINE1(0));
                clcd_print("Wait for", LINE2(0));
                clcd_print("secs", LINE2(12));
                // 1 min block if password wrong for max attempts
                sec = 60;
                // display time left
                while (sec > 0)
                {
                    clcd_putch(sec / 10 + '0', LINE2(9));
                    clcd_putch(sec % 10 + '0', LINE2(10));
                }
                // reset attempts left
                attempts_left = 3;
            }
            else
            {
                clear_screen();
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                // display attempts left for each wrong entry of password
                clcd_print("Wrong Password", LINE1(0));
                clcd_putch(attempts_left + '0', LINE2(0));
                clcd_print("attempts left", LINE2(2));
                __delay_ms(2000);
            }
            clear_screen();
            clcd_print("ENTER PASSWORD", LINE1(1));
            clcd_write(LINE2(4), INST_MODE);
            clcd_write(DISP_ON_CURSOR_ON, INST_MODE);
            i = 0;
            return_time = 5;
        }
    }
    return 0;
}

unsigned char menu_screen(unsigned char key, unsigned char reset_flag)
{
    // char *menu[] = {"View log", "Clear log", "Download log", "Set time", "Change passwd"};
    if (reset_flag == RESET_MENU)
    {
        clear_screen();
        menu_pos = 0;
        return_time = 5;
    }
    // return if idle for 5s
    if (return_time == 0)
        return RETURN_BACK_DEFAULT;
    
    // scroll up menu
    if (key == SW5 && menu_pos < 4)
    {
        menu_pos++; 
        clear_screen();
        return_time = 5;
    }
    // scroll down menu
    else if (key == SW4 && menu_pos > 0)
    {
        menu_pos--;
        clear_screen();
        return_time = 5;
    }
    
    // display * for current menu position
    if (menu_pos == 4)
    {
        clcd_print(menu[menu_pos - 1], LINE1(2));
        clcd_print(menu[menu_pos], LINE2(2));
        clcd_putch('*', LINE2(0));
    }
    else
    {
        clcd_print(menu[menu_pos], LINE1(2));
        clcd_print(menu[menu_pos + 1], LINE2(2));
        clcd_putch('*', LINE1(0));
    }
    return menu_pos;
}

static void get_saved_log(unsigned char view_pos)
{
    unsigned char i;
    
    unsigned char addr = (view_pos) * 10 + 5;   // 5, 15, 25 ....
    // get time from eeprom
    for (i = 0; i < 6; i++)
        saved_time[i] = read_external_eeprom(addr + i);
    saved_time[i] = '\0';
    
    // get event from eeprom
    saved_ev[0] = read_external_eeprom(addr + 6);
    saved_ev[1] = read_external_eeprom(addr + 7);
    saved_ev[2] = '\0';
    
    // get speed from eeprom
    saved_sp[0] = read_external_eeprom(addr + 8);
    saved_sp[1] = read_external_eeprom(addr + 9);
    saved_sp[2] = '\0';
}

static void print_log(unsigned char view_pos)
{
    // print the saved logs on clcd for view log menu
    clcd_putch(view_pos + '0', LINE2(0));
    clcd_putch(saved_time[0], LINE2(2));
    clcd_putch(saved_time[1], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(saved_time[2], LINE2(5));
    clcd_putch(saved_time[3], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(saved_time[4], LINE2(8));
    clcd_putch(saved_time[5], LINE2(9));
    
    clcd_putch(saved_ev[0], LINE2(11));
    clcd_putch(saved_ev[1], LINE2(12));
    
    clcd_putch(saved_sp[0], LINE2(14));
    clcd_putch(saved_sp[1], LINE2(15));
}

unsigned char view_log(unsigned char key, unsigned char reset_flag)
{
    static char view_pos;
    
    if (reset_flag == RESET_VIEW_LOG)
    {
        clear_screen();
        key = ALL_RELEASED;
        view_pos = 0;
        // display message
        if (log_flag)
        {
            clcd_print("# TIME     EV SP", LINE1(0));
        }
    }
    
    // check if at-least one log saves
    if (log_flag)
    {
        // scroll up logs
        if (key == SW4 && sw_p == SP)
        {
            if (view_pos > 0)
                view_pos--;
            else
                view_pos = log_count;
                
        }
        // scroll down logs
        else if (key == SW5 && sw_p == SP)
        {
            if (view_pos < log_count)
                view_pos++;
            else
                view_pos = 0;
        }
        // get the saved logs from eeprom
        get_saved_log(view_pos);
        // print logs on clcd
        print_log(view_pos);
    }
    else
    {
        clcd_print("No logs", LINE1(0));
        clcd_print("Available", LINE2(0));
    }
    
    // return to main menu for sw4 long press
    if (key == SW4 && sw_p == LP)
    {
        return_time = 5;
        return RETURN_BACK_MAIN_MENU;
    }
    // return to default screen for sw5 long press
    else if (key == SW5 && sw_p == LP)
        return RETURN_BACK_DEFAULT;
    
    return 0;
}

void clear_log(unsigned char reset_flag)
{
    if (reset_flag == RESET_LOG_MEM)
    {
        // reset flags and position tracker
        log_flag = 0;
        log_count = 0;
        pos = 0;
        // display message for 2s
        clcd_print("Logs cleared", LINE1(0));
        __delay_ms(2000);
    }
}

static void display_uart(unsigned char view_pos)
{
    putchar('\r');
    putchar('\n');
    // display sl no./position on teraterm
    putchar(view_pos + 1 + '0');
    putchar(' ');
    // display time on teraterm
    putchar(saved_time[0]);
    putchar(saved_time[1]);
    putchar(':');
    putchar(saved_time[2]);
    putchar(saved_time[3]);
    putchar(':');
    putchar(saved_time[4]);
    putchar(saved_time[5]);
    putchar(' ');
    // display event on teraterm
    putchar(saved_ev[0]);
    putchar(saved_ev[1]);
    putchar(' ');
    // display speed on teraterm
    putchar(saved_sp[0]);
    putchar(saved_sp[1]);
}

void download_log(void)
{
    unsigned char i = 1;
    // check if at-least one log saved
    if (log_flag)
    {
        // display title on teraterm
        puts("\r\n# TIME     EV SP\r\n");
        // get all saved logs and display on teraterm
        for (unsigned char i = 0; i < log_count; i++)
        {
            get_saved_log(i);
            display_uart(i);
        }
        
        // display message on clcd
        clcd_print("Logs Downloaded", LINE1(0));
    }
    else 
    {
        clcd_print("No logs", LINE1(0));
        clcd_print("available", LINE2(0));
        puts("\r\nNo logs available\r\n");
    }
    // display message for 2s
    __delay_ms(2000);
}

static void save_time(unsigned char hr, unsigned char min, unsigned char sec)
{
    hr = (unsigned char)(((hr / 10) << 4) | (hr % 10));
    min = (unsigned char)(((min / 10) << 4) | (min % 10));
    sec = (unsigned char)(((sec / 10) << 4) | (sec % 10));
    
    write_ds1307(HOUR_ADDR, hr);
    write_ds1307(MIN_ADDR, min);
    write_ds1307(SEC_ADDR, sec);
}

unsigned char set_time(unsigned char key, unsigned char reset_flag)
{
    static unsigned char hr, min, sec;
    static unsigned char field, blink_flag;
    static unsigned int delay;
    
    if (reset_flag == RESET_TIME)
    {
        get_time();
        // get time from rtc
        hr = ((clock_reg[0] >> 4) & 0x03);
        hr = hr * 10 + (clock_reg[0] & 0x03);
        
        min = ((clock_reg[1] >> 4) & 0x07);
        min = (min * 10) + ((clock_reg[1] & 0x0F));
        
        sec = ((clock_reg[2] >> 4) & 0x07);
        sec = (sec * 10) + ((clock_reg[2] & 0x0F));
        
        field = 0;
        blink_flag = 0;
        delay = 5;
        key = ALL_RELEASED;
        clcd_print("HH:MM:SS", LINE1(4));
    }
    
    // sw4 to increment time 
    if (key == SW4 && sw_p == SP)
    {
        if (field == 0)
        {
            sec++;
            if (sec >= 60)
                sec = 0;
        }
        else if (field == 1)
        {
            min++;
            if (min >= 60)
            {
                min = 0;
            }
        }
        else if (field == 2)
        {
            hr++;
            if (hr >= 24)
            {
                hr = 0;
            }
        }
        blink_flag = 0;
    }
    // sw5 to move field
    else if (key == SW5 && sw_p == SP)
    {
        if (field < 2)
            field++;
        else 
            field = 0;
    }
    // sw4 long press to return to main menu screen
    else if (key == SW4 && sw_p == LP)
    {
        return_time = 5;
        return RETURN_BACK_MAIN_MENU;
    }
    // sw5 long press to return to default screen after saving time
    else if (key == SW5 && sw_p == LP)
    {
        save_time(hr, min, sec);
        clear_screen();
        clcd_print("Time set", LINE1(0));
        clcd_print("Successful", LINE2(0));
        __delay_ms(2000);
        return RETURN_BACK_DEFAULT;
    }
    
    // blink seconds field
    if (field == 0)
    {
        clcd_putch((hr / 10) + '0', LINE2(4));
        clcd_putch((hr % 10) + '0', LINE2(5));
        clcd_putch(':', LINE2(6));
        clcd_putch((min / 10) + '0', LINE2(7));
        clcd_putch((min % 10) + '0', LINE2(8));
        clcd_putch(':', LINE2(9));

        if(--delay == 0)
        {
            delay = 5;
            if(blink_flag == 0)
            {
                clcd_putch((sec / 10) + '0', LINE2(10));
                clcd_putch((sec % 10) + '0', LINE2(11));
                blink_flag = 1;
            }
            else
            {
                clcd_print("  ", LINE2(10));
                blink_flag = 0;
            }
        }
    }
    // blink minutes field
    else if(field == 1)
    {
        clcd_putch((hr / 10) + '0', LINE2(4));
        clcd_putch((hr % 10) + '0', LINE2(5));
        clcd_putch(':', LINE2(6));
        clcd_putch(':', LINE2(9));
        clcd_putch((sec / 10) + '0', LINE2(10));
        clcd_putch((sec % 10) + '0', LINE2(11));

        if(--delay == 0)
        {
            delay = 5;
            if(blink_flag == 0)
            {
                clcd_putch((min / 10) + '0', LINE2(7));
                clcd_putch((min % 10) + '0', LINE2(8));
                blink_flag = 1;
            }
            else
            {
                clcd_print("  ", LINE2(7));
                blink_flag = 0;
            }
        }
    }
    // blink hours field
    if(field == 2)
    {
        clcd_putch(':', LINE2(6));
        clcd_putch((min / 10) + '0', LINE2(7));
        clcd_putch((min % 10) + '0', LINE2(8));
        clcd_putch(':', LINE2(9));
        clcd_putch((sec / 10) + '0', LINE2(10));
        clcd_putch((sec % 10) + '0', LINE2(11));
        
        if(--delay == 0)
        {
            delay = 5;
            if(blink_flag == 0)
            {
                clcd_putch((hr / 10) + '0', LINE2(4));
                clcd_putch((hr % 10) + '0', LINE2(5));
                blink_flag = 1;
            }
            else
            {
                clcd_print("  ", LINE2(4));
                blink_flag = 0;
            }
        }
    }
    return 0;
}

unsigned char change_passwd(unsigned char key, unsigned char reset_flag)
{
    static char new_pass[5], re_enter[5];
    static unsigned char flag, i;
    static unsigned char val;
    
    if (reset_flag == RESET_PASSWD)
    {
        key = ALL_RELEASED;
        i = 0;
        return_time = 5;
        // set flag to enter new password
        flag = PASS_ENTRY;
        clcd_print("Enter new passwd", LINE1(0));
        // turn on cursor
        clcd_write(LINE2(4), INST_MODE);
        clcd_write(DISP_ON_CURSOR_ON, INST_MODE);
    }
    
    // return to default screen if idle for 5s
    if (return_time == 0)
        return RETURN_BACK_DEFAULT;
    
    
    // sw4 - '1 , sw5 - '0'
    if ((key == SW4 || key == SW5) && i < 4)
    {
        clcd_putch('*', LINE2(4 + i));
        return_time = 5;
        val = (key == SW4) ? '1' : '0';
        
        // new password
        if (flag == PASS_ENTRY)
        {
            new_pass[i] = val;
        }
        // re-enter password
        else if (flag == PASS_RE_ENTRY)
        {
            re_enter[i] = val;
        }
        i++;
    }
    
    // re-enter password after new password is entered
    if (flag == PASS_ENTRY && i == 4)
    {
        i = 0;
        flag = PASS_RE_ENTRY;
        clear_screen();
        clcd_print("Renter passwd", LINE1(0));
        // turn on cursor
        clcd_write(LINE2(4), INST_MODE);
        clcd_write(DISP_ON_CURSOR_ON, INST_MODE);
    }
    
    // check if 4 digits of password re-entered
    if (flag == PASS_RE_ENTRY && i == 4)
    {
        new_pass[4] = '\0';
        re_enter[4] = '\0';
        // turn off timer2
        TMR2ON = 0;
        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
        clear_screen();
        
        // compare re-entered password with new password
        if (!strncmp(new_pass, re_enter, 4))
        {
            str_write_ext_eeprom(0x00, new_pass);
            clcd_print("Password change", LINE1(0));
            clcd_print("Successful", LINE2(0));
        }
        else
        {
            clcd_print("Password", LINE1(0));
            clcd_print("not matching", LINE2(0));
        }
        // display message for 2s and return to main menu screen
        __delay_ms(2000);
        i = 0;
        return RETURN_BACK_MAIN_MENU;
    }
    return 0;
}