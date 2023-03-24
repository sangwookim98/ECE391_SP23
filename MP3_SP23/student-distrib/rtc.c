#include "rtc.h"

#define VIDEO 0xB8000

#define NUM_ROWS 25
#define NUM_COLS 80

static char* video_mem = (char *)VIDEO;

void RTC_handler()
{
    outb(0x0C, 0x70);	/* Select Register C */
    inb(0x71);		/* Just throw away contents */
    printf("RTC INT! \n");
    /* Send EOI signal */
    send_eoi(8);
}


void test_interrupts_handler(){
    outb(0x0C, 0x70);	/* Select Register C */
    inb(0x71);		/* Just throw away contents */


    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }

    send_eoi(8);
}

void RTC_init() 
{
    /* Turning on IRQ8 (RTC) from OSDev */
    outb(REG_B, RTC);
    char prev = inb(CMOS);
    outb(REG_B, RTC);
    outb(prev | MASK, CMOS);
    enable_irq(8);
}
