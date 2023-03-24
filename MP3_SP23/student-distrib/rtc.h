#ifndef ___RTC_H            /* include for linking */
#include "types.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"

#ifndef ASM                 /* include this too */

#define REG_A 0x8A
#define REG_B 0x8B
#define RTC 0x70
#define CMOS 0x71
#define MASTER 0x20
#define MASK 0x40

/* Initialize RTC */
void RTC_handler();
void RTC_init();

/* IDT DEFINITION */

#endif
#endif
