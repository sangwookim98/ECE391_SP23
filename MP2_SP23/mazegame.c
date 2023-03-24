/*
 * tab:4
 *
 * mazegame.c - main source file for ECE398SSL maze game (F04 MP2)
 *
 * "Copyright (c) 2004 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Steve Lumetta
 * Version:       1
 * Creation Date: Fri Sep 10 09:57:54 2004
 * Filename:      mazegame.c
 * History:
 *    SL    1    Fri Sep 10 09:57:54 2004
 *        First written.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "blocks.h"
#include "maze.h"
#include "modex.h"
#include "text.h"
#include "module/tuxctl-ioctl.h"

// New Includes and Defines
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/io.h>
#include <termios.h>
#include <pthread.h>

#define BACKQUOTE 96
#define UP        65
#define DOWN      66
#define RIGHT     67
#define LEFT      68

/* for tux thread */
#define tux_up  239
#define tux_down 223
#define tux_left 191
#define tux_right 127
#define text_width 8
#define text_height 16
#define array_text 14


/*
 * If NDEBUG is not defined, we execute sanity checks to make sure that
 * changes to enumerations, bit maps, etc., have been made consistently.
 */
#if defined(NDEBUG)
#define sanity_check() 0
#else
static int sanity_check();
#endif


/* a few constants */
#define PAN_BORDER      5  /* pan when border in maze squares reaches 5    */
#define MAX_LEVEL       10 /* maximum level number                         */

/* Global variables */
int buttons;
int prev_buttons;

/* global variables for unveil around player function */
static int fruit_state;                /* hold state for few seconds */
int counter;
static int fruit_idx;

/* character array for fruit names to be used as transparent texts */
char fruits[fruit_name][len_name] = {"", "apple!", "grape!", "peach!", "strawberry!", "a banana!", "watermelon!", "YEAH DEW!", "hidden fruit!"};


/* outcome of each level, and of the game as a whole */
typedef enum {GAME_WON, GAME_LOST, GAME_QUIT} game_condition_t;

/* structure used to hold game information */
typedef struct {
    /* parameters varying by level   */
    int number;                  /* starts at 1...                   */
    int maze_x_dim, maze_y_dim;  /* min to max, in steps of 2        */
    int initial_fruit_count;     /* 1 to 6, in steps of 1/2          */
    int time_to_first_fruit;     /* 300 to 120, in steps of -30      */
    int time_between_fruits;     /* 300 to 60, in steps of -30       */
    int tick_usec;         /* 20000 to 5000, in steps of -1750 */
    
    /* dynamic values within a level -- you may want to add more... */
    unsigned int map_x, map_y;   /* current upper left display pixel */
} game_info_t;

static game_info_t game_info;

/* local functions--see function headers for details */
static int prepare_maze_level(int level);
static void move_up(int* ypos);
static void move_right(int* xpos);
static void move_down(int* ypos);
static void move_left(int* xpos);
static int unveil_around_player(int play_x, int play_y);
static void *rtc_thread(void *arg);
static void *keyboard_thread(void *arg);
/* for adding thread for tux */
static void *tux_thread(void *arg);



/* 
 * prepare_maze_level
 *   DESCRIPTION: Prepare for a maze of a given level.  Fills the game_info
 *          structure, creates a maze, and initializes the display.
 *   INPUTS: level -- level to be used for selecting parameter values
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: writes entire game_info structure; changes maze;
 *                 initializes display
 */
static int prepare_maze_level(int level) {
    int i; /* loop index for drawing display */
    
    /*
     * Record level in game_info; other calculations use offset from
     * level 1.
     */
    game_info.number = level--;

    /* Set per-level parameter values. */
    if ((game_info.maze_x_dim = MAZE_MIN_X_DIM + 2 * level) > MAZE_MAX_X_DIM)
        game_info.maze_x_dim = MAZE_MAX_X_DIM;
    if ((game_info.maze_y_dim = MAZE_MIN_Y_DIM + 2 * level) > MAZE_MAX_Y_DIM)
        game_info.maze_y_dim = MAZE_MAX_Y_DIM;
    if ((game_info.initial_fruit_count = 1 + level / 2) > 6)
        game_info.initial_fruit_count = 6;
    if ((game_info.time_to_first_fruit = 300 - 30 * level) < 120)
        game_info.time_to_first_fruit = 120;
    if ((game_info.time_between_fruits = 300 - 60 * level) < 60)
        game_info.time_between_fruits = 60;
    if ((game_info.tick_usec = 20000 - 1750 * level) < 5000)
        game_info.tick_usec = 5000;

    /* Initialize dynamic values. */
    game_info.map_x = game_info.map_y = SHOW_MIN;

    /* Create a maze. */
    if (make_maze(game_info.maze_x_dim, game_info.maze_y_dim, game_info.initial_fruit_count) != 0)
        return -1;
    
    /* Set logical view and draw initial screen. */
    set_view_window(game_info.map_x, game_info.map_y);
    for (i = 0; i < SCROLL_Y_DIM; i++)
        (void)draw_horiz_line (i);

    /* Return success. */
    return 0;
}

/* 
 * move_up
 *   DESCRIPTION: Move the player up one pixel (assumed to be a legal move)
 *   INPUTS: ypos -- pointer to player's y position (pixel) in the maze
 *   OUTPUTS: *ypos -- reduced by one from initial value
 *   RETURN VALUE: none
 *   SIDE EFFECTS: pans display by one pixel when appropriate
 */
static void move_up(int* ypos) {
    /*
     * Move player by one pixel and check whether display should be panned.
     * Panning is necessary when the player moves past the upper pan border
     * while the top pixels of the maze are not on-screen.
     */
    if (--(*ypos) < game_info.map_y + BLOCK_Y_DIM * PAN_BORDER && game_info.map_y > SHOW_MIN) {
        /*
         * Shift the logical view upwards by one pixel and draw the
         * new line.
         */
        set_view_window(game_info.map_x, --game_info.map_y);
        (void)draw_horiz_line(0);
    }
}

/* 
 * move_right
 *   DESCRIPTION: Move the player right one pixel (assumed to be a legal move)
 *   INPUTS: xpos -- pointer to player's x position (pixel) in the maze
 *   OUTPUTS: *xpos -- increased by one from initial value
 *   RETURN VALUE: none
 *   SIDE EFFECTS: pans display by one pixel when appropriate
 */
static void move_right(int* xpos) {
    /*
     * Move player by one pixel and check whether display should be panned.
     * Panning is necessary when the player moves past the right pan border
     * while the rightmost pixels of the maze are not on-screen.
     */
    if (++(*xpos) > game_info.map_x + SCROLL_X_DIM - BLOCK_X_DIM * (PAN_BORDER + 1) &&
        game_info.map_x + SCROLL_X_DIM < (2 * game_info.maze_x_dim + 1) * BLOCK_X_DIM - SHOW_MIN) {
        /*
         * Shift the logical view to the right by one pixel and draw the
         * new line.
         */
        set_view_window(++game_info.map_x, game_info.map_y);
        (void)draw_vert_line(SCROLL_X_DIM - 1);
    }
}

/* 
 * move_down
 *   DESCRIPTION: Move the player right one pixel (assumed to be a legal move)
 *   INPUTS: ypos -- pointer to player's y position (pixel) in the maze
 *   OUTPUTS: *ypos -- increased by one from initial value
 *   RETURN VALUE: none
 *   SIDE EFFECTS: pans display by one pixel when appropriate
 */
static void move_down(int* ypos) {
    /*
     * Move player by one pixel and check whether display should be panned.
     * Panning is necessary when the player moves past the right pan border
     * while the bottom pixels of the maze are not on-screen.
     */
    if (++(*ypos) > game_info.map_y + SCROLL_Y_DIM - BLOCK_Y_DIM * (PAN_BORDER + 1) && 
        game_info.map_y + SCROLL_Y_DIM < (2 * game_info.maze_y_dim + 1) * BLOCK_Y_DIM - SHOW_MIN) {
        /*
         * Shift the logical view downwards by one pixel and draw the
         * new line.
         */
        set_view_window(game_info.map_x, ++game_info.map_y);
        (void)draw_horiz_line(SCROLL_Y_DIM - 1);
    }
}

/* 
 * move_left
 *   DESCRIPTION: Move the player right one pixel (assumed to be a legal move)
 *   INPUTS: xpos -- pointer to player's x position (pixel) in the maze
 *   OUTPUTS: *xpos -- decreased by one from initial value
 *   RETURN VALUE: none
 *   SIDE EFFECTS: pans display by one pixel when appropriate
 */
static void move_left(int* xpos) {
    /*
     * Move player by one pixel and check whether display should be panned.
     * Panning is necessary when the player moves past the left pan border
     * while the leftmost pixels of the maze are not on-screen.
     */
    if (--(*xpos) < game_info.map_x + BLOCK_X_DIM * PAN_BORDER && game_info.map_x > SHOW_MIN) {
        /*
         * Shift the logical view to the left by one pixel and draw the
         * new line.
         */
        set_view_window(--game_info.map_x, game_info.map_y);
        (void)draw_vert_line (0);
    }
}

/* 
 * unveil_around_player
 *   DESCRIPTION: Show the maze squares in an area around the player.
 *                Consume any fruit under the player.  Check whether
 *                player has won the maze level.
 *   INPUTS: (play_x,play_y) -- player coordinates in pixels
 *   OUTPUTS: none
 *   RETURN VALUE: 1 if player wins the level by entering the square
 *                 0 if not
 *   SIDE EFFECTS: draws maze squares for newly visible maze blocks,
 *                 consumed fruit, and maze exit; consumes fruit and
 *                 updates displayed fruit counts
 */

static int unveil_around_player(int play_x, int play_y) {
    int x = play_x / BLOCK_X_DIM; /* player's maze lattice position */
    int y = play_y / BLOCK_Y_DIM;
    int i, j;            /* loop indices for unveiling maze squares */


    /* Check for fruit at the player's position. */
    fruit_state = check_for_fruit (x, y);

    if (fruit_state != 0) {
        fruit_idx = fruit_state;
        counter = 32 * 5;           /* 32 ticks per second, hold for 3 second */
    }

    /* Unveil spaces around the player. */
    for (i = -1; i < 2; i++)
        for (j = -1; j < 2; j++)
            unveil_space(x + i, y + j);
        unveil_space(x, y - 2);
        unveil_space(x + 2, y);
        unveil_space(x, y + 2);
        unveil_space(x - 2, y);

    /* Check whether the player has won the maze level. */
    return check_for_win (x, y);
}

#ifndef NDEBUG
/* 
 * sanity_check 
 *   DESCRIPTION: Perform checks on changes to constants and enumerated values.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if checks pass, -1 if any fail
 *   SIDE EFFECTS: none
 */
static int sanity_check() {
    /* 
     * Automatically detect when fruits have been added in blocks.h
     * without allocating enough bits to identify all types of fruit
     * uniquely (along with 0, which means no fruit).
     */
    if (((2 * LAST_MAZE_FRUIT_BIT) / MAZE_FRUIT_1) < NUM_FRUIT_TYPES + 1) {
        puts("You need to allocate more bits in maze_bit_t to encode fruit.");
        return -1;
    }
    return 0;
}
#endif /* !defined(NDEBUG) */

// Shared Global Variables
int quit_flag = 0;
int winner= 0;
int next_dir = UP;
int play_x, play_y, last_dir, dir;
int move_cnt = 0;
int fd;                                 /* fd for keyboard interrupt */
unsigned long data;
static struct termios tio_orig;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

/* added 5th March */
/* added for TUX thread */
static pthread_cond_t cv;
int fd_thread;

/* for CP 2*/
int prev_x;
int text_y_offset;



/* added on 4th March for TUX thread 
*
 * tux_thread
 *   DESCRIPTION: Thread that handles TUX inputs
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

 /* connection to current button and past button */
static void *tux_thread(void *arg) {
    // Break only on win or quit input - '`'
    while (winner == 0) {
        if (quit_flag == 1) {
            break;
        }

        prev_buttons = buttons; 
        ioctl(fd_thread, TUX_BUTTONS, &buttons);                    /* fixed lag */
        pthread_mutex_lock(&mtx);  
        // Get button Input        
        while (buttons == 0xFF) {
            pthread_cond_wait(&cv, &mtx);
        }

        /* TODO: past button check present button */
        switch(buttons) {
            case tux_up:
                next_dir = DIR_UP;
                break;
            case tux_down:
                next_dir = DIR_DOWN;
                break;
            case tux_right:
                next_dir = DIR_RIGHT;
                break;
            case tux_left:
                next_dir = DIR_LEFT;
                break;
            }
            
        pthread_mutex_unlock(&mtx);
    }
    return 0;
}


/*
 * keyboard_thread
 *   DESCRIPTION: Thread that handles keyboard inputs
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void *keyboard_thread(void *arg) {
    char key;
    int state = 0;
    // Break only on win or quit input - '`'
    while (winner == 0) {        
        // Get Keyboard Input
        key = getc(stdin);
        
        // Check for '`' to quit
        if (key == BACKQUOTE) {
            quit_flag = 1;
            break;
        }
        
        // Compare and Set next_dir
        // Arrow keys deliver 27, 91, ##
        if (key == 27) {
            state = 1;
        }
        else if (key == 91 && state == 1) {
            state = 2;
        }
        else {    
            if (key >= UP && key <= LEFT && state == 2) {
                pthread_mutex_lock(&mtx);
                switch(key) {
                    case UP:
                        next_dir = DIR_UP;
                        break;
                    case DOWN:
                        next_dir = DIR_DOWN;
                        break;
                    case RIGHT:
                        next_dir = DIR_RIGHT;
                        break;
                    case LEFT:
                        next_dir = DIR_LEFT;
                        break;
                }
                pthread_mutex_unlock(&mtx);
            }
            state = 0;
        }
    }

    return 0;
}

/* some stats about how often we take longer than a single timer tick */
static int goodcount = 0;
static int badcount = 0;
static int total = 0;

/*
 * rtc_thread
 *   DESCRIPTION: Thread that handles updating the screen
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void *rtc_thread(void *arg) {
    int ticks = 0;                              /* to be used to calculate clock */
    int level;
    int ret;
    int open[NUM_DIRS];
    int need_redraw = 0;
    int goto_next_level = 0;

    /* CP 2*/
    unsigned char text[text_width * text_height * array_text];            /* pixel * max. length of string */
    unsigned char background[text_width * text_height * array_text];

    // Loop over levels until a level is lost or quit.
    for (level = 1; (level <= MAX_LEVEL) && (quit_flag == 0); level++) {
        /* use sprintf, strcat to put words into status bar */
        /* level time fruit */
        /* This reset time at each level */
        total = 0;
        /* getting the needed texts to print out */
        int curr_level = level;                         /* obtaining current level while being inside the level */
        char str_level[10];                             /* used 10 as the number to allocate enough space for charcters to be stored in an array */
        sprintf(str_level, "%d", curr_level);           /* converting to string or char type of curr_level using sprintf */
        
        int fruit;
        fruit = get_nfruit();                           /* gets number of fruits to get */
        char str_fruit[10];
        
        /* tick => continuously increases when game starts, rate = 32 times per 1 sec */
        /* making a stopwatch: in seconds */
        char str_time[10];                              /* used 10 as the number to allocate enough space for charcters to be stored in an array */
        unsigned int seconds, minutes;
        minutes = (total * 32);                         /* 32Hz per sec for each tick */
        seconds = minutes % 60;                         /* updates seconds by % with 60 since 60 seconds in 1 minute */
        
        change_wallcolor(level);

        /*LEVEL 1   2 Fruits   06:40*/
        /* using strcat for level*/
        char a[10], b[10];
        strcpy(b, "Level ");
        strcpy(a, str_level);

        strncat(b, a, 8);

        // // /* using strcat for fruit*/
        // char c[10], d[10];
        sprintf(str_fruit, "   %d Fruit", fruit);           /* converting to string or char type of curr_level using sprintf */
        /* change to strncat */
        strcat(b, str_fruit);

        // /* using strcat for level + fruit */
        sprintf(str_time, "   %.2d:%.2d", minutes, seconds);           /* converting to string or char type of curr_level using sprintf */
        strcat(b, str_time);

        // /* centering of text in status bar */

        text_instatus(b, level);

        // Prepare for the level.  If we fail, just let the player win.
        if (prepare_maze_level(level) != 0)
            break;
        goto_next_level = 0;

        // Start the player at (1,1)
        play_x = BLOCK_X_DIM;
        play_y = BLOCK_Y_DIM;

        // move_cnt tracks moves remaining between maze squares.
        // When not moving, it should be 0.
        move_cnt = 0;

        // Initialize last direction moved to up
        last_dir = DIR_UP;

        // Initialize the current direction of motion to stopped
        dir = DIR_STOP;
        next_dir = DIR_STOP;

        // Show maze around the player's original position
        (void)unveil_around_player(play_x, play_y);

        
        // masking for player 
        draw_masked_player(play_x, play_y, get_player_block(last_dir), get_player_mask(last_dir), temp_buff);
        show_screen();
        draw_full_block(play_x, play_y, temp_buff);

        // get first Periodic Interrupt
        ret = read(fd, &data, sizeof(unsigned long));

        while ((quit_flag == 0) && (goto_next_level == 0)) {
            // Wait for Periodic Interrupt
            ret = read(fd, &data, sizeof(unsigned long));
        
            // Update tick to keep track of time.  If we missed some
            // interrupts we want to update the player multiple times so
            // that player velocity is smooth
            ticks = data >> 8;    

            total += ticks;

            // If the system is completely overwhelmed we better slow down:
            if (ticks > 8) ticks = 8;

            if (ticks > 1) {
                badcount++;
            }
            else {
                goodcount++;
            }

            while (ticks--) {
                /* added in 5th March 12am: tux thread */
                /* following the week 7 discussion slides */
                prev_buttons = buttons; 

                ioctl(fd_thread, TUX_BUTTONS, &buttons);
                
                // Lock the mutex
                pthread_mutex_lock(&mtx);

                if (buttons != 0xFF && prev_buttons != buttons) {
                    pthread_cond_signal(&cv);
                }

                // Check to see if a key has been pressed
                if (next_dir != dir) {
                    // Check if new direction is backwards...if so, do immediately
                    if ((dir == DIR_UP && next_dir == DIR_DOWN) ||
                        (dir == DIR_DOWN && next_dir == DIR_UP) ||
                        (dir == DIR_LEFT && next_dir == DIR_RIGHT) ||
                        (dir == DIR_RIGHT && next_dir == DIR_LEFT)) {
                        if (move_cnt > 0) {
                            if (dir == DIR_UP || dir == DIR_DOWN)
                                move_cnt = BLOCK_Y_DIM - move_cnt;
                            else
                                move_cnt = BLOCK_X_DIM - move_cnt;
                        }
                        dir = next_dir;
                    }
                }
                // New Maze Square!
                if (move_cnt == 0) {
                    // The player has reached a new maze square; unveil nearby maze
                    // squares and check whether the player has won the level.
                    if (unveil_around_player(play_x, play_y)) {
                        pthread_mutex_unlock(&mtx);
                        goto_next_level = 1;
                        break;
                    }
                
                    // Record directions open to motion.
                    find_open_directions (play_x / BLOCK_X_DIM, play_y / BLOCK_Y_DIM, open);
        
                    // Change dir to next_dir if next_dir is open 
                    if (open[next_dir]) {
                        dir = next_dir;
                    }
    
                    // The direction may not be open to motion...
                    //   1) ran into a wall
                    //   2) initial direction and its opposite both face walls
                    if (dir != DIR_STOP) {
                        if (!open[dir]) {
                            dir = DIR_STOP;
                        }
                        else if (dir == DIR_UP || dir == DIR_DOWN) {    
                            move_cnt = BLOCK_Y_DIM;
                        }
                        else {
                            move_cnt = BLOCK_X_DIM;
                        }
                    }
                }

                // Unlock the mutex
                pthread_mutex_unlock(&mtx);
        
                if (dir != DIR_STOP) {
                    // move in chosen direction
                    last_dir = dir;
                    move_cnt--;    
                    switch (dir) {
                        case DIR_UP:    
                            move_up(&play_y);    
                            break;
                        case DIR_RIGHT: 
                            move_right(&play_x); 
                            break;
                        case DIR_DOWN:  
                            move_down(&play_y);  
                            break;
                        case DIR_LEFT:  
                            move_left(&play_x);  
                            break;
                    }

                    need_redraw = 1;
                }
            /* CP2 */
            prev_x = play_y;

            if (counter > 0) {
                /* functions below done in an ordered manner */
                /* NOTE: 7, 4, and 8 are offset numbers that I used to test to roughly center transparent text */
                draw_masked_player(play_x, play_y, get_player_block(last_dir), get_player_mask(last_dir), temp_buff);       /* calls draw function to mask player onto screen, stored into temp_buff */
                copy_full_block(play_x - 7 * FONT_WIDTH + 4,  play_y - FONT_HEIGHT - 8, background);                        /* copies block at that position into buffer called background */
                copy_full_block(play_x - 7 * FONT_WIDTH + 4, play_y - FONT_HEIGHT - 8, text);                               /* copies block at that position into buffer called text */
                fruit_text_to_graphic(fruits[fruit_idx], text, background);                                                 /* calls function to draw the transparent text on the background buffer */

                build_buffer_to_block(play_x - 7 * FONT_WIDTH + 4, play_y - FONT_HEIGHT - 8, text);                         /* function to make text buffer from the current position block */
                show_screen();  
                build_buffer_to_block(play_x - 7 * FONT_WIDTH + 4, play_y - FONT_HEIGHT - 8, background);                   /* at that position, function called to build buffer from the background buffer */
                draw_full_block(play_x, play_y, temp_buff);                                                                 /* draw block using the temp_buff for player */
                counter--;                
            }
            else {
                    /* from checkpoint 1*/
                    draw_masked_player(play_x, play_y, get_player_block(last_dir), get_player_mask(last_dir), temp_buff);
                    show_screen();
                    draw_full_block(play_x, play_y, temp_buff);    
            }

            }       

            /* constant updates on fruit and time */
            /* getting the needed texts to print out */
            int curr_level = level;                         /* obtaining current level while being inside the level */
            char str_level[10];
            sprintf(str_level, "%d", curr_level);           /* converting to string or char type of curr_level using sprintf */
            
            int fruit;
            fruit = get_nfruit();                           /* gets number of fruits to get */
            char str_fruit[10];
            
            /* TIME CONVERSION */
            /* tick => continuously increases when game starts, rate = 32 times per 1 sec */
            /* making a stopwatch: in seconds */
            char str_time[10];
            unsigned int seconds, minutes, display_sec;
            seconds = (total / 32);                         /* 32Hz per sec for each tick */
            minutes = seconds / 60;                         /* updates seconds since 1 min = 60s */
            display_sec = seconds % 60;                         
            
            /* Putting time into TUX emulator */
            unsigned int min_10, min_1s, sec_10, sec_1s, tux_timer;         /* variables to be used to display LED in TUX emulator */
            min_10 = minutes / 10;                          /* used 10 as the number to show the minutes in multiples of 10 */
            min_1s = minutes % 10;                          /* mod 10 gives you the remainder of minutes (1 ~ 9) */

            sec_10 = display_sec / 10;                      /* used 10 as the number to show the minutes in multiples of 10 */
            sec_1s = display_sec % 10;                      /* mod 10 gives you the remainder of minutes (1 ~ 9) */

            tux_timer = 0;                                  /* initialziation for tux timer */

            /* in case of using the 10s place of the minute */
            if(min_10 == 0) {
                tux_timer += 15;                            /* add x0F */
            }

            else {
                tux_timer += min_10;
            }

            tux_timer = tux_timer << 4;                     /* bitshift to the left 4 times indicate moving on to the next LED */

            tux_timer += min_1s;                            /* constant update on the 1s place of the minutes*/
            tux_timer = tux_timer << 4;                     /* bitshift to the left 4 times indicate moving on to the next LED */

            tux_timer += sec_10;                            /* constant update on the tens place of the seconds */
            tux_timer = tux_timer << 4;                     /* bitshift to the left 4 times indicate moving on to the next LED */

            tux_timer += sec_1s;                            /* constant update on the units place of the seconds */

            /* buglog: fixing the correct decimal point: originally was at x080F */
            ioctl(fd_thread, TUX_SET_LED, 0x040F << 16 | tux_timer);                 /* setting the LEDS to show on emulator */

            /*LEVEL 1   2 Fruits   06:40*/
            /* using strcat (string concatenation) for level*/
            char a[10], b[10];

            strcpy(b, "Level ");
            strcpy(a, str_level);
            strncat(b, a, 8);

            // /* using strcat for fruit*/
            if (fruit == 0) {
                sprintf(str_fruit, "   %d Fruits", fruit);           /* converting to string or char type of curr_level using sprintf */
            }
            else {
                sprintf(str_fruit, "   %d Fruit", fruit);            /* converting to string or char type of curr_level using sprintf */
            }
            
            /* change to strncat */
            strcat(b, str_fruit);

            // /* using strcat for level + fruit */
            sprintf(str_time, "   %.2d:%.2d", minutes, display_sec);           /* converting to string or char type of curr_level using sprintf */
            strcat(b, str_time);

            /* centering of text in status bar */
            text_instatus(b, level);

            /* CP2: random changing character */
            change_playercolor();
            change_outlinecolor(level);


        }    
    }
    /* winning condition to exit game */
    if (quit_flag == 0)
        winner = 1;
    
    return 0;
}

/*
 * main
 *   DESCRIPTION: Initializes and runs the two threads
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int main() {
    int ret;
    struct termios tio_new;
    unsigned long update_rate = 32; /* in Hz */

    pthread_t tid1;
    pthread_t tid2;

    /* pthread_t type variable added for tux thread */
    pthread_t tid3; 

    // Initialize RTC
    fd = open("/dev/rtc", O_RDONLY, 0);

    /* fd_thread needed for tux */
    fd_thread = open("/dev/ttyS0", O_RDWR | O_NOCTTY);
    int ldisc_num = N_MOUSE;
    ioctl(fd_thread, TIOCSETD, &ldisc_num);
    /* BUG LOG: FORGOT TO INITIALIZE THREAD -> caused things to not reset and values keep carrying over after compile (timer) until reset pressed
    first compile won't move after pressing reset, etc */
    ioctl(fd_thread, TUX_INIT, 0);

    
    // Enable RTC periodic interrupts at update_rate Hz
    // Default max is 64...must change in /proc/sys/dev/rtc/max-user-freq
    ret = ioctl(fd, RTC_IRQP_SET, update_rate);    
    ret = ioctl(fd, RTC_PIE_ON, 0);

    // Initialize Keyboard
    // Turn on non-blocking mode
    if (fcntl(fileno(stdin), F_SETFL, O_NONBLOCK) != 0) {
        perror("fcntl to make stdin non-blocking");
        return -1;
    }
    
    // Save current terminal attributes for stdin.
    if (tcgetattr(fileno(stdin), &tio_orig) != 0) {
        perror("tcgetattr to read stdin terminal settings");
        return -1;
    }
    
    // Turn off canonical (line-buffered) mode and echoing of keystrokes
    // Set minimal character and timing parameters so as
    tio_new = tio_orig;
    tio_new.c_lflag &= ~(ICANON | ECHO);
    tio_new.c_cc[VMIN] = 1;
    tio_new.c_cc[VTIME] = 0;
    if (tcsetattr(fileno(stdin), TCSANOW, &tio_new) != 0) {
        perror("tcsetattr to set stdin terminal settings");
        return -1;
    }

    // Perform Sanity Checks and then initialize input and display
    if ((sanity_check() != 0) || (set_mode_X(fill_horiz_buffer, fill_vert_buffer) != 0)){
        return 3;
    }

    // Create the threads
    pthread_create(&tid1, NULL, rtc_thread, NULL);
    pthread_create(&tid2, NULL, keyboard_thread, NULL);

    /* create thread for TUX */
    pthread_create(&tid3, NULL, tux_thread, NULL);

    // Wait for all the threads to end
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    // termination of tux thread 
    pthread_cancel(tid3);

    // Shutdown Display
    clear_mode_X();
    
    // Close Keyboard
    (void)tcsetattr(fileno(stdin), TCSANOW, &tio_orig);
        
    // Close RTC
    close(fd);

    // Close fd used for thread  
    close(fd_thread);

    // Print outcome of the game
    if (winner == 1) {    
        printf("You win the game! CONGRATULATIONS!\n");
    }
    else if (quit_flag == 1) {
        printf("Quitter!\n");
    }
    else {
        printf ("Sorry, you lose...\n");
    }

    // Return success
    return 0;
}
