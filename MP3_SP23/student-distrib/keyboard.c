#include "keyboard.h"

// static int shift = 0;
// static int caps = 0;
/* keyboard input as array */
//fix this
char lowercase_array[256] = {'~', '~', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '~', '~', 'q', 'w', 
                'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[' , ']', '~', '~', 'a', 's', 'd', 'f',
                'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '~', '~', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
                '.', '/', '~', '*'
                };               

void keyboard_init() 
{
    enable_irq(1);      /* raise IRQ with its corresponding PIC */
}

void keyboard_handler() 
{
    uint32_t scan;
    uint32_t scan_off;
    uint8_t character;
    int flag; 

    // printf("Entering keyboard handler.\n");

    scan = inb(0x60);   //get scancode from port - from OSDev

    
    // if(scan == 0x36 || scan == 0x2A) shift = 1; //shift is pressed
    // if(scan == 0xAA || scan == 0xB6) shift = 0; //shift is released
    // if(scan == 0x3A) {
    //     if(caps == 0) caps = 1;
    //     else if(caps == 1) caps = 0;
    // }

    // if(shift == 0 && caps == 0) { //shift and caps are off
        //character = lowercase_array[scan]; //get correct character from array
    // }
    // else if(shift == 1 && caps == 0) { //shift is pressed
    //     character = shift_array[scan]; //get correct character from array
    // } 
    // else if(shift == 0 && caps == 1) { //caps lock is on
    //     character = caps_array[scan]; //get correct character from array
    // } 
    // else if(shift == 1 && caps == 1) {
    //     if(scan >= 0x10 && scan <= 0x19) {
    /* to get released keys in the keyboard */
    if(scan >= 0x81)
    {
        flag = 1;
        scan_off = scan - 0x80;
        character = (uint8_t)lowercase_array[scan_off]; //get correct character from array
    }
    else
    {
        flag = 0;
        character = (uint8_t)lowercase_array[scan]; //get correct character from array
    }

    

    //     }
    //     else if(scan >= 0x1E && scan <= 0x26) {
    //         character = lowercase_array[scan]; //get correct character from array
    //     }
    //     else if(scan >= 0x2C && scan <= 0x32) {
    //         character = lowercase_array[scan]; //get correct character from array
    //     }
    //     else if(scan >= 0x10 && scan <= 0x19) {
    //         character = lowercase_array[scan]; //get correct character from array
    //     }    
    //     else character = shift_array[scan]; //get correct character from array
    // } 

    if(flag == 0)
    {
        printf("You typed: %c\n", character); //print the character
    }

    else
    {
        /*Ask about why release key does not show up*/
        printf("You released: %c\n", character); //print the character
        // printf("You released a key\n"); //print the character
    }
    send_eoi(1);
}


//     uint32_t scan;
//     uint8_t character;

//     printf("Entering keyboard handler.\n");

//     scan = inb(0x60);   //get scancode from port - from OSDev

//     // if(scan == 0x36 || scan == 0x2A) shift = 1; //shift is pressed
//     // if(scan == 0xAA || scan == 0xB6) shift = 0; //shift is released
//     // if(scan == 0x3A) {
//     //     if(caps == 0) caps = 1;
//     //     else if(caps == 1) caps = 0;
//     // }

//     // if(shift == 0 && caps == 0) { //shift and caps are off
//         //character = lowercase_array[scan]; //get correct character from array
//     // }
//     // else if(shift == 1 && caps == 0) { //shift is pressed
//     //     character = shift_array[scan]; //get correct character from array
//     // } 
//     // else if(shift == 0 && caps == 1) { //caps lock is on
//     //     character = caps_array[scan]; //get correct character from array
//     // } 
//     // else if(shift == 1 && caps == 1) {
//     //     if(scan >= 0x10 && scan <= 0x19) {
//     character = (uint8_t)lowercase_array[scan]; //get correct character from array
//     //     }
//     //     else if(scan >= 0x1E && scan <= 0x26) {
//     //         character = lowercase_array[scan]; //get correct character from array
//     //     }
//     //     else if(scan >= 0x2C && scan <= 0x32) {
//     //         character = lowercase_array[scan]; //get correct character from array
//     //     }
//     //     else if(scan >= 0x10 && scan <= 0x19) {
//     //         character = lowercase_array[scan]; //get correct character from array
//     //     }    
//     //     else character = shift_array[scan]; //get correct character from array
//     // } 

//     printf("You typed: %c\n", character); //print the character
//     send_eoi(1);
// }


