#include "KBSend.h"

// KB_press simule l'appui sur le clavier de la toucher v (code decimal)
// a est le tableau des touches qui ont été pressées et b des touches qui viennent d'être pressées.
void KB_press(bool* a, bool b, short v)
{
    if(b && !*a)
        {
            keybd_event(v,0,KEYEVENTF_EXTENDEDKEY,0);
            *a = 1;
        }
        if(*a && !b)
        {
            keybd_event(v,0,KEYEVENTF_KEYUP,0);
            *a = 0;
        }
}

void KB_enter()
{
    keybd_event(13,0,KEYEVENTF_EXTENDEDKEY,0);
    keybd_event(13,0,KEYEVENTF_KEYUP,0);
}
