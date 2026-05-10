#ifndef CAR_BLACK_BOX_H
#define	CAR_BLACK_BOX_H

void display_dashboard(char event[], unsigned char speed);
void log_event(char event[], unsigned char speed);
unsigned char login(unsigned char key, unsigned char reset_flag);
unsigned char menu_screen(unsigned char key, unsigned char reset_flag);
unsigned char view_log(unsigned char key, unsigned char reset_flag);
void clear_log(unsigned char reset_flag);
void download_log(void);
unsigned char set_time(unsigned char key, unsigned char reset_flag);
unsigned char change_passwd(unsigned char key, unsigned char reset_flag);
void clear_screen(void);

#endif	/* CAR_BLACK_BOX_H */

