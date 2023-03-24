.data					# section declaration

        # Useful offset constants for accessing members of a 
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
        
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret

/* # Interface: C-style, Stack-based argument 
    c form: void mp1 rtc tasklet (unsigned long)
#   Inputs:  %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
#   Registers: Clobbers %eax, %ebx, %edx
 */
mp1_rtc_tasklet:
    /*  1. every time RTC interrupt called, mp1_rtc_tasklet called. 
        2. traverse down mp1_list_head list, examining each mp1_blink_struct
        3. function decrements down countdown of struct
        4. if countdown reaches zero -> tasklet examine status part.
            -> if status == 1: location currently has on_char character
            -> if status == 0: location currently has off_char character
        5. interchange character using poke function, interchange status between 1 and 0
            -> similar to the mp_poke section used in add function
        6. update countdown field by copying value from opposite length field to countdown
        off -> on: copy on_length to countdown, vice-versa
        function returns when reaching end of list
    */
    /* stack set up */
    push %ebp
    movl %esp, %ebp

    /*  traverse down the struct */
    /* callee saved registers <- pop this at the very end, at ending label*/  
    pushl %ebx
    pushl %edi 
    pushl %esi

    pushl %eax
    pushl %ecx
    pushl %edx
    pushfl

    /* getting head pointer */ 
    movl mp1_list_head, %ebx                        /* ebx as the pointer to the head */

    /* traverse to all struct */
    check_headptr:
        testl %ebx, %ebx                            /* use test for self, only uses flag for branching */
        jnz valid_headptr                           /* checks if headpointer is NULL */
        jz end_linkedlist

    valid_headptr:
        movw COUNTDOWN(%ebx), %dx                   /* dx used to save countdown for each struct */
        /* decrement countdown */
        decw %dx
        movw %dx, COUNTDOWN(%ebx)        
        cmpw $0, %dx
        jne next_list

        /* countdown is 0 */ 
        xorw %dx, %dx                               /* clear dx register just in case */
        movw STATUS(%ebx), %dx                      /* dx as the register with status */
        cmpw $0, %dx
        je status_zero

        /* status is one */
        movw $0, %dx                                /* changing status to 0 from 1 */
        movw %dx, STATUS(%ebx)                      /* update STRUCT */

        /* Using mp1_poke - Getting location */
        xorl %eax, %eax
        movw LOCATION(%ebx), %ax
        shll $1, %eax                               /* doubled due to 2 memory locations corresponding to each character, location refers to number of 
                                                    characters, helps us get memory address by multiplying 2 to the addresses */

        /* %cl register: 8 bits => 2 bytes
        getting from on_char with location as on 
        Status is ONE */
        movb OFF_CHAR(%ebx), %cl                    /* copying off_charaacter values to cl to print out */
        call mp1_poke   

        movw OFF_LENGTH(%ebx), %dx                  /* updating countdown using off character length */
        movw %dx, COUNTDOWN(%ebx)

    next_list:
        movl NEXT(%ebx), %ebx                       /* ebx used to visit every struct */
        cmpl $0, %ebx
        jne check_headptr                           /* check next head pointer if valid */
        je end_linkedlist

    status_zero:
        movw $1, %dx                                /* changing status to 1 from 0 */
        movw %dx, STATUS(%ebx)                      /* update STRUCT */

        /* Using mp1_poke - Getting location */
        xorl %eax, %eax
        movw LOCATION(%ebx), %ax
        shll $1, %eax                               /* doubled due to 2 memory locations corresponding to each character, location refers to number of 
                                                    characters, helps us get memory address by multiplying 2 to the addresses */

        movb ON_CHAR(%ebx), %cl
        call mp1_poke                               /* %cl register: 8 bits => 2 bytes
                                                    getting from on_char with location as on */
    
        movw ON_LENGTH(%ebx), %dx                   /* update on length to the countdown for on character */
        movw %dx, COUNTDOWN(%ebx)
        jmp next_list                               /* next iteration */

    end_linkedlist:                                 /* stack teardown */
        popfl
        popl %edx
        popl %ecx
        popl %eax

        popl %edi
        popl %esi
        popl %ebx
        leave        
    ret

/*  Interface: C-style, Stack based Argument 
    Inputs: %ecx  - cmd values corresponding core functions to jump to
    Outputs: if cmd is not in between values of 0 and 3 (inclusive), outputs -1
    Purpose: uses cmd values and decides which function to go to next using the jump table
    Registers: clobbers %eax register 

*/
mp1_ioctl:
/* check for the invalid  for cmd */
    movl 8(%esp), %ecx                           /* saving cmd to ecx register */

    movl 4(%esp), %edx                           /* savind arg to a register edx */ 

    /* checking for debugging */
    /* xorl %eax, %eax
        addl $5, %ecx */
    cmpl $0, %edx
    je invalid

    cmpl $3, %ecx                                /* ecx - 3, eax need to be 0 1 2 3  */ 
    jg invalid
    cmpl $0, %ecx                                /* ecx - 0, eax need to be 0 1 2 3 */
    jl invalid

/* cmd within 0 to 3 inclusive, valid case */ 
    jmp *jump_table(, %ecx, 4)                   /* determine which function t operform */

invalid: 
    xorl %eax, %eax                              /* clear eax for storing result */
    subl $1, %eax                                /* -1 for cmd */
	jmp dispatch_ending

dispatch_ending:
    ret


jump_table:
        .long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync

/* ------------------------------------------------------------------------------------------------------------------------------
  Interface: C-style, Stack based arguments
  C-form: int mp1_ioctl_add (unsigned long arg)
        Input:   pointer to the location in the struct
        Outputs: returns 0 if successful
                 returns -1 if either memory allocation did not work, mp1_copy_from_user failed to copy everything, or location is out of range
        Use: using arg, the ioctl_add function 
                1) allocates space to copy struct into
                2) uses mp1_copy_from_user to copy struct
                3) checks location to display characters
                4) uses mp1_poke to display characters

        registers used: %eax, %esi, %ecx
*/ 

mp1_ioctl_add:
/* second function to modify: 
step 1. uses mp1_malloc function to dynamically allocate memory */
/* todo: base case error for malloc */ 
    pushl %ebp
    movl %esp, %ebp 

    /* callee saved registers <- pop this at the very end, at ending label*/  
    pushl %ebx 
    pushl %edi 
    pushl %esi

    movl 8(%ebp), %esi                                      /* using esi reigster to store arg parameter */
    cmpl $0, %esi                                           /* checking if esi register is not NULL */
    je  arg_error


    /* callee parameter to push -> unsigned long size */
    pushl $STRUCT_SIZE
    call mp1_malloc                                         /* calls mp1_malloc function */
    addl $4, %esp
    
    /* eax register check if failed or not */
    movl %eax, %ecx                                         /* ecx has the result of mp1_malloc function */
    cmpl $0, %ecx
    je arg_error                                            /* checks if result is NULL */ 

    /* malloc worked, restore callee saved registers */
    # popfl

/* step 2. copy entire mp1_blink_struct from user space structure to newly allocated memory - use mp1_copy_from_user */

    /* callee has 3 parameters-> void *to, const void *from, unsigned long n */
    /* to: result from malloc, ebx*/
    pushl $STRUCT_SIZE                                      /* structsize */
    pushl %esi                                              /* esi: from has arg */
    pushl %ecx                                              /* ecx: to has malloc result */

    call mp1_copy_from_user
    popl %ecx
    addl $8, %esp                                           /* pop 3 parameters from copy_from_user */

    /* not able to copy all => return other than 0: copy failed, return -1 */
    cmpl $0, %eax
    jne malloc_failed

/*  5.   make call to mp1_poke with correct register parameters to display character on screen 
    will turn location "on". After countdown RTC intterupts, mp1_rtc_tasklet will turn location "off"
    return 0 if successful add was executed */
    /* base case for poke, finding location 
    start:0 
    end: 80 * 25  */
    cmpw $1999, LOCATION(%ecx)                              /* $1999 is a magic number that is used to represent the maximum number for location range.
                                                                document has said to check for location within range 0 to 80*25-1 where 80*25-1 equals to 1999 */
    jg malloc_failed                                        /* location greater than 1999, memory allocated space need to free */
    cmpw $0, LOCATION(%ecx)
    jl malloc_failed                                        /* location less than 0, memory allocated space need need to be free */ 

/* CHECK step 3. set countdown field equal to on_length field, set status to 1, accessing elements using ecx register */    
    movw ON_LENGTH(%ecx), %si
    movw %si, COUNTDOWN(%ecx)
    movw $1, STATUS(%ecx)
        
/* 4. insert this structure to head of linked list using mp1_list_head pointer */
    /* ecx has the result of malloc */ 
    /* using mp1_list_head */
    movl mp1_list_head, %esi
    movl %ecx, mp1_list_head                /* result of malloc pointing to the head */
    movl %esi, NEXT(%ecx)

    /* Getting location */
    xorl %eax, %eax
    movw LOCATION(%ecx), %ax
    shll $1, %eax                           /* doubled due to 2 memory locations corresponding to each character, location refers to number of 
                                            characters, helps us get memory address by multiplying 2 to the addresses */

    /* %cl register: 8 bits => 2 bytes
    getting from on_char with location as on */
    movb ON_CHAR(%ecx), %cl                 /* preapring cl register to store on screen character to display using mp1_poke function */
    call mp1_poke                           /* function called to display on screen characater */
    
    /* success */
    movl $0, %eax
    jmp ending                              /* jump to stack teardown */

arg_error:                                  /* arg_error for checking if arg is valid*/
    /* return -1 */
    movl $-1, %eax
    jmp ending                              /* jump to stack teardown */

malloc_failed:                              /* malloc failed, but because it already called mp1_malloc function, anything that comes after calling malloc needs to be free using mp1_free function */
    /* free assigned memory */
    pushl %ecx                              /* ecx register has the result of mp1_malloc function */
    call mp1_free                           /* function called to display on screen characater */
    popl %ecx   

    movl $-1, %eax                          /* malloc failed, need to return -1 to return in function */
    jmp ending                              /* jump to stack teardown and return */

ending:                                     /* stack teardown and return */
    popl %esi
    popl %edi
    popl %ebx
    leave  
    ret

/* ------------------------------------------------------------------------------------------------------------------------------
  Interface: C-style, Stack based argument
  c-form: int mp1_ioctl_remove (unsigned long arg)
        Inputs: arg (location)
        Outputs: returns 0 if successful
                 returns -1 if there is not match between target location and the address in struct of linked list
        Use: given an arg (location), remove the struct the location belongs in from the struct of linked list
                1) find the address of location 
                2) traverse through linked list
                3) if there is a match, remove from linked list, then free that location from the struct
                   else, return -1

        registers used: %eax, %ebx, %ecx, %edx, %esi
*/ 
mp1_ioctl_remove:
    pushl %ebp
    movl %esp, %ebp

    /* callee saved registers <- pop this at the very end, at ending label*/  
    pushl %ebx
    pushl %edi 
    pushl %esi

    movl 8(%ebp), %edx                      /* edx register has the arg parameter */

    pushl %edx
    call helperfunc_search                  /* using helper function to get target location to remove */
    addl $4, %esp

    movl %eax, %ebx                         /* ebx has the location of address to remove, after helper function */

    cmpl $-1, %eax                          /* checking result of helper function if it returns -1 or address */
    je no_element_toremove

/* case 1: pointer at head, first head of the struct */
remove_checkfirst_struct:   
    movl mp1_list_head, %ecx                /* ecx has the head pointer to a struct, will be used as curr_ptr */
    cmpl $0, %ecx
    je no_element_toremove                  /* pointer at the very first struct */

    /* if target == mp1_list_head */
    cmpl %ecx, %ebx                         /* target vs head pointer */
    je element_found_firststruct

/* case 2: for other nodes in the struct other than the first node */
remove_updatepointer:
    /* update head pointer to next struct after not finding */
    xorl %eax, %eax                         /* clear eax */
    # movl NEXT(%ecx), %eax                 /* save currptr -> eax register */

    movl NEXT(%ecx), %ecx                   /* update current pointer -> next = current pointer */
    cmpl $0, %ecx                           /* check if current pointer is NULL */
    jne remove_checkothernode               /* currptr is at the middle */
    je no_element_toremove                  /* currptr finish traversal, return -1 */

remove_checkothernode:
    cmpl NEXT(%ecx), %ebx                   /* getting previous node: currptr->next vs target */         
    je element_found_otherstruct            /* location found */
    jne remove_updatepointer                /* location not found, go to next head of the struct */

element_found_firststruct: 
    xorl %eax, %eax
    movl NEXT(%ecx), %eax                   /* eax has cur -> next */
    movl %eax, mp1_list_head                /* esi as the temporary node */
    pushl %ecx
    call mp1_free                           /* free allocated space */
    addl $4, %esp                           /* pop parameter to mp1_free function */
    xorl %eax, %eax
    jmp remove_finish

element_found_otherstruct:
    xorl %esi, %esi
    movl NEXT(%ebx), %esi                   /* target->next to esi register */
    movl %esi, NEXT(%ecx)                   /* esi has ecx->next */

    pushl %ebx                              /* free target pointer */
    call mp1_free           
    addl $4, %esp                           /* pop parameters 
    xorl %eax, %eax                         /* remove done, return 0 */
    jmp remove_finish

no_element_toremove:                        
    xorl %eax, %eax                         /* clear eax to return -1 in function upon return */
    subl $1, %eax

remove_finish:                              /* stack teardown, and return */
    popl %edi
    popl %ebx
    leave
	ret


/* ------------------------------------------------------------------------------------------------------------------------------
  Interface: C-style, Stack based argument
  c-form: helperfunc_search (unsigned long arg)
        Input:   arg (location)
        Outputs: returns address to that location if there is a match between the arg and in any part of the struct (linked list)
                 returns -1 if there is not match between target location and the address in struct of linked list
        Use:    given an arg (location)
                1) get the head pointer of the linked list
                2) compare with 16 bits the current pointer at location with the arg (input)
                3) if there is a match, return that as the address
                   else if there is no match, return -1

        registers used: %eax, %ebx, %esi
*/
helperfunc_search:
    pushl %ebp
    movl %esp, %ebp

    /* callee saved registers <- pop this at the very end, at ending label*/  
    pushl %ebx
    pushl %edi 
    pushl %esi 

    movw 8(%ebp), %si                              /* si has the given input, random location */
    
    /* set up for location in struct */
    movl mp1_list_head, %ebx                        /* start of struct to ebx register */
    cmpl $0, %ebx                                   /* check if head pointer is null */
    je helperfunc_endofstruct

check_location:
    cmpw LOCATION(%ebx), %si                     /* check if matching location */
    jne next_locationstruct 
    je location_match

location_match:
    movl %ebx, %eax
    jmp helperfunc_end

next_locationstruct:
    movl NEXT(%ebx), %ebx               /* update ebx to the next struct's head pointer */
    cmpl $0, %ebx                       /* check if pointer is valid and not null */
    jne check_location

helperfunc_endofstruct:                 /* once iterated through end of struct in linked list, return -1 */
    xorl %eax, %eax
    subl $1, %eax

helperfunc_end:                         /* popping parameters, end of function */
    popl %esi
    popl %edi
    popl %ebx
    leave        
	ret    

/* ------------------------------------------------------------------------------------------------------------------------------
  Interface: C-style, Stack based argument
  c-form: int mp1 ioctl find (unsigned long arg)
        Input:   arg (location)
        Outputs: returns address to that location if there is a match between the arg and in any part of the struct (linked list)
                 returns -1 if there is not match between target location and the address in struct of linked list
        Use:    given an arg (location)
                1) get the head pointer of the linked list
                2) compare with 16 bits the current pointer at location with the arg (input)
                3) if there is a match, return that as the address
                   else if there is no match, return -1

        registers used: %eax, %ebx, %esi
 */
mp1_ioctl_find:
    /*  edx : arg from dispatcher */
    pushl %ebp
    movl %esp, %ebp

    movl 8(%ebp), %edx                      /* save arg into edx */
    /* callee saved registers <- pop this at the very end, at ending label*/  
    pushl %ebx
    pushl %edi 
    pushl %esi

    /* checking arg within struct */
    movl $STRUCT_SIZE, %eax
    pushl %eax                              /* eax with 16 into malloc function */
    call mp1_malloc
    addl $4, %esp
    movl %eax, %ebx
    cmpl $0, %ebx                           /* check for malloc if malloc failed */

    je find_malloc_error

    /* calling for the mp1_copy_from_user (to, from, n)*/


    xorl %eax, %eax                         /* clear eax register */
    movl $STRUCT_SIZE, %eax                 /* struck size -> edi */
    movl %edx, %esi                         /* edx arg -> esi */
                                            /* ebx: result of helper function */

    /* pushing 3 parameters to mp1_copy_from_user */
    pushl %eax                              /* pushed as the n parameter */
    pushl %esi                              /* pushed arg as the from parameter */
    pushl %ebx                              /* pushed as the to parameter */
    call mp1_copy_from_user   
    popl %ebx                               /* pop 3 parameters used for copy_from_user function*/
    popl %esi
    popl %eax
    
    cmpl $0, %eax                           /* comparing with eax which is the result of the called function */
    jne find_copy_failed                    /* if eax returns a value that isn't zero, copy has failed */

    xorl %esi, %esi                         /* esi cleared to use as a temporary register */
    movl LOCATION(%ebx), %esi
    pushl %ebx                              /* clear target location */
    call mp1_free       
    addl $4, %esp                           /* popped target parameter */

    /* copy check */
    cmpl $0, %esi                           /* checking if temporary register does not have any values on it */
    jne find_copy_failed                    /* copy failed if esi isn't 0 */

    /* saving caller-saved registers */

    pushl %esi                              
    call helperfunc_search
    addl $4, %esp

    xorl %esi, %esi                                 /* clearing esi to store result of helper function to esi register */
    movl %eax, %esi                                 /* address to location */

    test %eax, %esi                                 /* checking between eax and esi register */
    js find_notfound                                /* if there is any nonzero value between the two - helper function did not find location */


    movl $STRUCT_SIZE, %ecx                         /* moving STRUCT_SIZE to ecx register */
    pushl %ecx                                      /* 16: to put into  n para */
    pushl %esi                                      /* address location to be put into from parameter */
    pushl %edx                                      /* edx: arg to put into the to parameter */
    call mp1_copy_to_user           
    addl $12, %esp                                  /* pop 3 parameters pushed into mp1_copy_to_user */

    /* copy check */
    cmpl $0, %eax                                   /* successful copy means 0 in %eax register */
    jne find_copy_failed

    /* copy success */
    xorl %eax, %eax
    jmp find_finish                                 /* find finished */

find_malloc_error:                                  /* malloc error: malloc returned a NULL pointer */
    xorl %eax, %eax
    subl $1, %eax
    jmp find_finish                                 /* jump to stack teardown and function return */

find_copy_failed:                                   /* copy failed */
    xorl %eax, %eax
    subl $1, %eax
    jmp find_finish                                 /* jump to stack teardown and function return */

find_notfound:                                      /* no matching value between arg and anywhere on the linked list */
    xorl %eax, %eax
    subl $1, %eax
 
find_finish:                                        /* successful for find function, stack teardown and return function */
    popl %esi
    popl %edi
    popl %ebx
    leave        
	ret   

/* ------------------------------------------------------------------------------------------------------------------------------
  Interface: C-style, Stack based argument
  c-form: int mp1_ioctl_sync (unsigned long arg)
        Input:   arg (location)
        Outputs: returns 0 if synchronization between on and off characters are shown
                 returns -1 if there is no synchronization
        Use:    given an arg (location)
                1) get two existing location 
                2) traverse linked list to find a matching location with the first and second blinking characters
                3) copy over timing information from first to second location
                4) use mp1_poke to display character on second location

        registers used: %eax, %ebx, %ecx, %edx, %edi, %esi
 */

mp1_ioctl_sync:
    pushl %ebp                         /* stack build up */
    movl %esp, %ebp

    /* callee saved registers <- pop this at the very end, at ending label*/  
    pushl %ebx
    pushl %edi 
    pushl %esi

    movl 8(%ebp), %ebx                  /* ebx as the register to store location */

    /* edx has 32-bit arg */
    xorl %ecx, %ecx                     /* clear ecx register */
    cmpl $0, %ebx                       /* arg check, if invalid, return -1 */
    je arg_invalid

    movw %bx, %cx                       /* second location in cx */
    shrl $16, %ebx                      /* shift 16times in edx */
    movw %bx, %dx                       /* first location in dx */

    /* finding the address of first location */
    pushl %edx
    call helperfunc_search
    popl %edx                           
    cmpl $-1, %eax                      /* checking if helper function return -1 (no match for arg and location in struct) */
    je sync_loc_notfound
    movl %eax, %esi                     /* copy 1st result to esi register */

    /* finding the address of second location */
    pushl %ecx
    call helperfunc_search
    popl %ecx
    cmpl $-1, %eax                      /* checking if helper function return -1 (no match for arg and location in struct) */
    je sync_loc_notfound
    movl %eax, %edi                     /* copy 2nd result to edi register */
    
    xorl %eax, %eax
    /* copying information: first to second element */
    movw ON_LENGTH(%esi), %ax           /* using ax register to temporary store on screen character length of first location */
    movw %ax, ON_LENGTH(%edi)           /* then using ax register to replace value for on screen character length of second location */


    movw OFF_LENGTH(%esi), %ax          /* using ax register to temporary store off screen character length of first location */
    movw %ax, OFF_LENGTH(%edi)          /* then using ax register to replace value for off screen character length of second location */


    movw COUNTDOWN(%esi), %ax           /* using ax register to temporary store countdown of first location */
    movw %ax, COUNTDOWN(%edi)           /* then using ax register to replace countdown of second location */



    movw STATUS(%esi), %ax              /* using ax register to temporary store status of first location */
    movw %ax, STATUS(%edi)              /* then using ax register to replace status of second location */

    /* call mp1_poke */
    xorl %eax, %eax                     
    movw STATUS(%edi), %ax              /* extracting status value */

    cmpl $0, %eax   
    je Second_locStatuszero             /* if status is 0: off character status */
    jmp Second_locStatusone             /* if status is 1: on character status */

Second_locStatuszero:
    xorl %eax, %eax
    movw LOCATION(%edi), %ax            /* get the 16 bit value for off_character location into ax register */
    shlw $1, %ax                        /* double ax register */
    xorl %ecx, %ecx

    movb OFF_CHAR(%edi), %cl            /* storing off screen character to cl to output to screen */
    call mp1_poke                       /* function to call for putting output on screen */
    xorl %eax, %eax
    jmp sync_finish                     /* synchronized success */

Second_locStatusone:
    xorl %eax, %eax
    movw LOCATION(%edi), %ax            /* get the 16 bit value for on_character location into ax register */
    shlw $1, %ax
    xorl %ecx, %ecx

    movb ON_CHAR(%edi), %cl             /* storing on screen character to cl to output to screen */
    call mp1_poke                       /* function to call for putting output on screen */
    xorl %eax, %eax
    jmp sync_finish                     /* synchronized success */

sync_loc_notfound:                      /* location for sync isnt found, return -1 */
    xorl %eax, %eax
    subl $1, %eax

sync_finish:                            /* sync success, stack teardown + return */
    popl %esi
    popl %edi
    popl %ebx
    leave
	ret

arg_invalid:                            /* arg is not valid, stack teardown + return */
    xorl %eax, %eax
    subl $1, %eax
    popl %esi
    popl %edi
    popl %ebx

    leave
    ret

.end
