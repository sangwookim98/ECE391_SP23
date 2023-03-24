#include "idt.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"

/*  handler_irqX()
*   Description: Handler function for IRQ number X 
*   Input: None
*   Output: None
*   Return Value: 
*   Side Effects: Prints interrupt request
*/

void handler_div0()
{  
    printf("Divide by 0 Exception");
    while(1);
}

void handler_debug() 
{
    printf("Debug Exception");
    while(1);
}

void handler_NMI()
{
    printf("NMI Interrupt");
    while(1);
}

void handler_breakpoint() 
{
    printf("Breakpoint Exception");
    while(1);
}   

void handler_overflow() 
{
    printf("Overflow Exception");
    while(1);
}

void handler_bound() 
{
    printf("Bound Range Exceeded Exception");
    while(1);
}

void handler_invalid_op() 
{
    printf("Invalid Opcode Exception");
    while(1);
}

void handler_unavliable_device() 
{
    printf("Device not Avaliable Exception");
    while(1);
}

void handler_double_fault() 
{
    printf("Double Fault Exception");
    while(1);
}

void handler_coproccessor() 
{
    printf("Coproccesor Segment Overrun");
    while(1);
}

void handler_invalid_tss() 
{
    printf("Invalid TSS Exception");
    while(1);
}

void handler_segment_npresent() 
{
    printf("Segement Not Present");
    while(1);
}

void handler_stack_fault() 
{
    printf("Stack Fault Exception");
    while(1);
}

void handler_genprotect() 
{
    printf("General Protection Exception");
    while(1);
}

void handler_page_fault() 
{
    printf("Page Fault Exception");
    while(1);
}

void handler_FPU() 
{
    printf("FPU Floaiting Point Error");
    while(1);
}


void handler_alignment()
{
    printf("Alignment Check Exception");
    while(1);
}

void handler_machine()
{
    printf("Machine Check Exception");
    while(1);   
}

void handler_SIMD()
{
    printf("SIMD Floating-Point Exception");
    while(1);
}

void handler_call()
{
    printf("A system call was called.");
    while(1);
}

/*  IDT_int()
*   Description: Sets the entry for each interrupt vectors and its corresponding device
*                pertaining to the intterupt vector number. 
*   Input: None
*   Output: None
*   Return Value: None
*   Side effects: Creates entries for interrupt vectors/
*/
void IDT_init()
{
    /*set the flags to 0 from 0x30 to 0xFF*/
    int i;
    
    for (i = 0; i < NUM_VEC; i++)
    {
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 0;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size      = 1;
        idt[i].reserved0 = 0;
        idt[i].dpl       = 0;
        idt[i].present   = 1;

        if (i == 128) idt[i].dpl = 3;
    }
    
    //Indicies were determiend throught the IDT table in the lecture notes.
    SET_IDT_ENTRY(idt[0], &handler_div0);
    SET_IDT_ENTRY(idt[1], &handler_debug);
    SET_IDT_ENTRY(idt[2], &handler_NMI);
    SET_IDT_ENTRY(idt[3], &handler_breakpoint);
    SET_IDT_ENTRY(idt[4], &handler_overflow);
    SET_IDT_ENTRY(idt[5], &handler_bound);
    SET_IDT_ENTRY(idt[6], &handler_invalid_op);
    SET_IDT_ENTRY(idt[7], &handler_unavliable_device);
    SET_IDT_ENTRY(idt[8], &handler_double_fault);
    SET_IDT_ENTRY(idt[9], &handler_coproccessor);
    SET_IDT_ENTRY(idt[10], &handler_invalid_tss);
    SET_IDT_ENTRY(idt[11], &handler_segment_npresent);
    SET_IDT_ENTRY(idt[12], &handler_stack_fault);
    SET_IDT_ENTRY(idt[13], &handler_genprotect);
    SET_IDT_ENTRY(idt[14], &handler_page_fault);
    SET_IDT_ENTRY(idt[16], &handler_div0);
    SET_IDT_ENTRY(idt[17], &handler_debug);
    SET_IDT_ENTRY(idt[18], &handler_NMI);
    SET_IDT_ENTRY(idt[19], &handler_breakpoint);
    SET_IDT_ENTRY(idt[33], &keyboard_handler_wrapper);              /* wrapper called using assembly linkage */
    SET_IDT_ENTRY(idt[0x28], &rtc_handler_wrapper);                 /* called using assembly linkage */
    SET_IDT_ENTRY(idt[128], &handler_call);  

    /* for CHECKPOINT 1 - need 0x80 */
    
}
