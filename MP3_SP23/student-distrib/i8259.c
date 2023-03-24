/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init()
{
    
    /* Masking Interrupts */
    outb(0xFF, PIC1_DATA); /* for PIC 1 DATA */
    outb(0xFF, PIC2_DATA); /* for PIC 2 DATA */

    /* MASTER PIC - ICW assignment */
    outb(ICW1, MASTER_8259_PORT); /* intialization of primary PIC */
    outb(ICW2_MASTER, MASTER_8259_PORT + 1); /* Sets IR0-7 from 0x20 to 0x27 */
    outb(ICW3_MASTER, MASTER_8259_PORT + 1);
    outb(ICW4, MASTER_8259_PORT + 1);


    /* SECONDARY PIC - ICW assignment */
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1); /* Sets IR0-7 from 0x20 to 0x27 */
    outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);
    outb(ICW4, SLAVE_8259_PORT + 1);

    /* Masking Interrupts */
    outb(0xFF, PIC1_DATA); /* for PIC 1 DATA */
    outb(0xFF, PIC2_DATA); /* for PIC 2 DATA */

    enable_irq(2); /* Never disable IRQ2 from the slave PIC */

}

/* Enable (unmask) the specified IRQ */
/* if secondary PIC is used, leave IRQ2 of primary PIC */
/* OSdev: clear mask */
void enable_irq(uint32_t irq_num) 
{
    /* Clear interrupts */
    cli();
    /* Master is enabled */
    if (irq_num < 8)
    {
        outb((inb(PIC1_DATA) & ~(1 << irq_num)), PIC1_DATA);
    }
    /* Slave and Master enabled */
    else    
    {
        irq_num -= 8; 
        outb((inb(PIC2_DATA) & ~(1 << irq_num)), PIC2_DATA);
    }
    /* Restore interrupts */
    sti();
}

/* Disable (mask) the specified IRQ */
/* Ask if we have to disable irq2 if we disable a slave irq / edge case needed? */
void disable_irq(uint32_t irq_num) 
{
    /* Clear interrupts */
    cli();
    /* Master is enabled */
    if (irq_num == 2)
        return;

    if (irq_num < 8)
    {
        outb((inb(PIC1_DATA) | (1 << irq_num)), PIC1_DATA);
    }

    /* slave is enabled, technically so is master (IRQ2) */
    else
    {
        irq_num -= 8; 
        outb((inb(PIC2_DATA) | (1 << irq_num)), PIC2_DATA);
    }
    /* Restore interrupts */
    sti();
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) 
{
    /* Clear interrupts */
    cli();
    if(irq_num >= 8)
    {
        /* EOI to PIC2 command port*/
        outb(EOI | (irq_num - 8), PIC2_CMD);
        outb(EOI | 2, PIC1_CMD);
    } 
    
    else 
    {
        /* EOI to PIC1 command port*/
        outb(EOI | irq_num, PIC1_CMD);  
    }
    /* Restore interrupts */
    sti();              
}
