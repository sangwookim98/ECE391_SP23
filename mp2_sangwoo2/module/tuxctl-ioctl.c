/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

/* TODO: Check when to use flags, if flags are set correctly */

int ackn = 1;                           /* flag for ACKNOWLEDGE */
/* List of global variables used */
/* This variable is for mthe handle packets in tuxctl_handle_packet */
int finalized_packet;

static int intialize_tux(struct tty_struct* tty);
static int set_LED(struct tty_struct* tty, unsigned long arg);
static int button_seq(int arg);


/* this global variable is for the LED display */
/* List of numbers needed:
0: E7
1: 06
2: CB
3: 8F
4: 2E
5: AD
6: ED
7: 86
8: EF
9: AE
A: EE
B: 6D
C: E1
D: 4F
E: E9
F: E8
*/
unsigned char buffer_hex[16] = {0xE7, 0x06, 0xCB, 0x8F, 0x2E, 0xAD, 0xED, 0x86, 0xEF, 0xAE, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0x00};

/* for reset */
unsigned long prev_LED; 

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */

 /* TO DO */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	unsigned char save_left, save_down, b_masked, maintain_right_up, get_down, get_left, b_mask;


    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

	switch(a) {
		case MTCP_ACK :
            ackn = 1;
            break;

		case MTCP_RESET : 
            /* on reset */
            intialize_tux(tty);
            set_LED(tty, prev_LED);

            ackn = 1; 
            break;

		case MTCP_BIOC_EVENT :
			/* all masks needed to create button */
			maintain_right_up = 0x09;
			get_down = 0x04;
			get_left = 0x02;
            b_mask = 0x0F;

			save_left = c & get_left;					/* gets left bit*/
			save_down = c & get_down;					/* gets down bit */

            b_masked = b & b_mask;

			finalized_packet = c & maintain_right_up;				/* gets rid of existing down and left */

			/* got the right down left up as first four bits */
			finalized_packet = finalized_packet << 4;	/* moves 4 bits to the left*/
            save_left = save_left << 4;
            save_down = save_down << 4;

            /* switch positions */
			save_left = save_left << 1;
			save_down = save_down >> 1;


			finalized_packet = finalized_packet + save_down + save_left + b_masked;		
            break;
	}

    // printk("packet : %x %x %x\n", a, b, c);
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:
        return intialize_tux(tty);

	case TUX_BUTTONS:
        return button_seq(arg);

	case TUX_SET_LED:
    /* changed in 5th march morning */
        return set_LED(tty, arg);

	case TUX_LED_ACK:
	case TUX_LED_REQUEST:
	case TUX_READ_LED:
	default:
	    return -EINVAL;
    }
}

/* ioctl #1: tux_init */
static int intialize_tux(struct tty_struct* tty) {
    /* anything stored in buffer -> send to TUX */
    int init_to_tux;
    /* MTCP Opcodes used: MTCP_BIOC_ON, MTCP_LED_SET */
    unsigned char tux_init_buffer[2];
    tux_init_buffer[0] = MTCP_BIOC_ON;
    tux_init_buffer[1] = MTCP_LED_USR;                                      /* allows the LED to be display user set values */

    init_to_tux = tuxctl_ldisc_put(tty, tux_init_buffer, 2);

    /* Checking if writing to device was success */
    if (init_to_tux > 0) {
        return -EINVAL;
    }

    ackn = 1;
    return 0;
}



/* ioctl #2: tux_set_led */
static int set_LED(struct tty_struct* tty, unsigned long arg) {
	/* Given an argument in the form of 
	aaaa aaaa bbbb bbbb cccc cccc dddd dddd 
	      dp        L#    L4   L3   L2   L1
	bits 24 to 27: specify whether corresponding decimal poitns should be turned on 
	bits 0  to 15: decimal number to display on Hexadecimal 
	bits 16 to 19: specify which LED to turn on 
	*/

    /* buffer to send to TUX */
	unsigned char LEDdisplay_toTUX[6];

	/* Variables declared */
	int LED1;
	int LED2;
	int LED3;
	int LED4;
    int select_LED;
	int select_decimal;
    int i;
    int check_LED, check_dp;
    
    /* spam case check */
    if (ackn == 0) {
        return -1;
    }

    prev_LED = arg;

	/* Isolating LEDs out of arg */
	LED1 = arg & 0x0000000F;
	
	LED2 = arg & 0x000000F0;
	LED2 = LED2 >> 4;								/* 4 is used to shift 2nd LED to the least significant bit place*/

	LED3 = arg & 0x00000F00;						
	LED3 = LED3 >> 8;								/* 8 is used to shift 2nd LED to the least significant bit place*/

	LED4 = arg & 0x0000F000;
	LED4 = LED4 >> 12;								/* 12 is used to shift 2nd LED to the least significant bit place*/

	/* Getting which LED to turn on */
	select_LED = arg & 0x000F0000;					/* bitmask to only have that portion shown*/
    select_LED = select_LED >> 16;
	/* Getting which decimal to put bits onto*/ 
	select_decimal = arg & 0x0F000000;
    select_decimal = select_decimal >> 24;

    /* storing LEDs into buffer */
    LEDdisplay_toTUX[0] = MTCP_LED_SET;             
    LEDdisplay_toTUX[1] = select_LED;             
    LEDdisplay_toTUX[2] = buffer_hex[(int)LED1];             
    LEDdisplay_toTUX[3] = buffer_hex[(int)LED2];
    LEDdisplay_toTUX[4] = buffer_hex[(int)LED3];
    LEDdisplay_toTUX[5] = buffer_hex[(int)LED4];

    /* check for  DP and which LED used */


    for(i = 2; i < 6; i++) {
        /* 1. checking for which LED is on or off */
        check_LED = select_LED & 0x01;
        /* check for LED */
        if (check_LED == 0) {
            LEDdisplay_toTUX[i] = 0x00;
        }

        /* 2. checking for which decimal is on or off */
        check_dp = select_decimal & 0x01;
        /* check for decimal point */
        if (check_dp == 1) {
            LEDdisplay_toTUX[i] = LEDdisplay_toTUX[i] | 0x10;
        }
        /* bug log: did not update decimal bit select. could not change which decimal to display in mazegame */
        select_decimal = select_decimal >> 1;

    }


    /* tuxctl_ldisc_put() is used to write to TUX from computer */
    /* opcode used: MTCP_LED_SET */
    tuxctl_ldisc_put(tty, LEDdisplay_toTUX, 6);	

    /* save LEDs after reset */
    ackn = 0;
    return 0;
}


/* ioctl #3 -> tux_button */
static int button_seq(int arg) {
    /* check if pointer is invalid */
    int check;
    check = copy_to_user((int*)arg, &finalized_packet, sizeof(finalized_packet));

    if (check == 0) {
        return 0;
    }

    return -EINVAL;

}
