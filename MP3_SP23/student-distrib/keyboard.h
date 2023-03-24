#ifndef ___KEYBOARD_H
#define ___KEYBOARD_H
#include "types.h"
#include "lib.h"
#include "i8259.h"

#ifndef ASM  

void keyboard_init();
void keyboard_handler();


// unsigned char shift_array[256] = {};                 /* FOR FUTURE */
// unsigned char caps_array[256] = {};                 /* FOR FUTURE */

#endif
#endif
