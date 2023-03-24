# /* interrupt handlers calling the interrupt gate convention */

#define ASM 1

.text
.align 4

.globl keyboard_handler_wrapper

/*  keyboard_handler_wrapper
*   Description: Keyboard handler function for keyboard inputs, 
*   Input: None
*   Output: None
*   Return Value: 
*   Side Effects: Prints keyboard interrupts into kernel 
*/
keyboard_handler_wrapper:
    pushal
    call keyboard_handler
    popal
    iret

    
.globl rtc_handler_wrapper

/*  rtc_handler_wrapper
*       Description: RTC handler function for RTC interrupts, 
*       Input: None
*       Output: None
*       Return Value: 
*       Side Effects: Prints whatever characters are in RTC. 
*/
rtc_handler_wrapper:
    pushal
    call RTC_handler 
    /* call test_interrupts_handler */
    popal
    iret
