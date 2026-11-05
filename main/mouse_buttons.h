#ifndef MOUSE_BUTTONS_H
#define MOUSE_BUTTONS_H


extern volatile int left_btn_state;
extern volatile int right_btn_state;
void button_task(void* arg);

#endif