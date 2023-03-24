#ifndef ___IDT_H            /* include for linking */
#define ___IDT_H
#include "types.h"
#include "x86_desc.h"

#ifndef ASM                 /* include this too */

extern idt_desc_t idt[NUM_VEC];

/* Initialize IDT */
void IDT_init();

/* IDT DEFINITION */
void handler_div0();
void handler_debug();
void handler_NMI();
void handler_breakpoint();
void handler_overflow();
void handler_bound();
void handler_invalid_op();
void handler_unavliable_device();
void handler_double_fault();
void handler_coproccessor();
void handler_invalid_tss();
void handler_segment_npresent(); 
void handler_stack_fault();
void handler_genprotect(); 
void handler_page_fault();
void handler_FPU();
void handler_alignment();
void handler_machine();
void handler_SIMD(); 
void handler_call();

/* from the idt_asm.S file */
extern void keyboard_handler_wrapper(void);     /* keyboard interrupts */
extern void rtc_handler_wrapper(void);          /* rtc interrupts */

#endif
#endif
