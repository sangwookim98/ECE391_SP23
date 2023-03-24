#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

#define PASS 1
#define FAIL 0
#define MEM_ZERO 0
#define INT_MAX 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 - 1  

#define BEFORE_VIDEO 0xB7FFF
#define VIDEO 0xB8000
#define AFTER_VIDEO 0xB8001
#define MID_VIDEO 0xB8800
#define BEFORE_END_VIDEO 0xB8FFF
#define END_VIDEO 0xB9000
#define AFTER_END_VIDEO 0xB9500

#define BEFORE_KERNEL 0x003FFFFF
#define KERNEL 0x00400000
#define AFTER_KERNEL 0x00400001
#define MID_KERNEL 0x00700000
#define BEFORE_END_KERNEL 0x007FFFFF
#define END_KERNEL 0x00800000
#define AFTER_END_KERNEL 0x00850000

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* --------------------------------------------- START OF CHECKPOINT 1 TESTS ---------------------------------------------------------------------------------------*/
// add more tests here
void divide_by_0_test()
{
	uint32_t x = 5;
	uint32_t y = 0;
	x = x / y;
}

/* paging_test_0 to paging_test_4 checks location of the VIDEO */

/* paging_test_0 
* description: simple test that checks the contents at 0 MB
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_0: at 0 MB */
void paging_test_0()
{
	char* ref;
	char t;
	ref = (char *)MEM_ZERO;
	t = *ref;
	return;
}

/* paging_test_1 
* description: simple test that checks the contents at 1 byte above VIDEO (0xB7FFF)
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_1: before VIDEO */
void paging_test_1()
{
	char* ref;
	char t;
	ref = (char *)BEFORE_VIDEO;
	t = *ref;
	return;
}

/* paging_test_2
* description: simple test that checks the contents at VIDEO (0xB8000)
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_2: at VIDEO */
void paging_test_2()
{
	char* ref;
	char t;
	ref = (char *)VIDEO;
	t = *ref;
	return;
}

/* paging_test_3
* description: simple test that checks the contents at 1 byte below VIDEO (0xB8001)
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_3: below VIDEO */
void paging_test_3()
{
	char* ref;
	char t;
	ref = (char *)AFTER_VIDEO;
	t = *ref;
	return;
}

/* paging_test_4
* description: simple test that checks the contents in the middle between VIDEO (0xB8001) and END_VIDEO (0xB900)
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_4: in middle of VIDEO memory */
void paging_test_4()
{
	char* ref;
	char t;
	ref = (char *)MID_VIDEO;
	t = *ref;
	return;
}

/* paging_test_5
* description: simple test that checks the contents one byte above END_VIDEO (0xB8FFF)
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_5: before end of VIDEO memory */
void paging_test_5()
{
	char* ref;
	char t;
	ref = (char *)BEFORE_END_VIDEO;
	t = *ref;
	return;
}

/* paging_test_6
* description: simple test that checks the contents one byte at END_VIDEO (0xB9000), 4KB
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_6: at end of VIDEO memory */
void paging_test_6()
{
	char* ref;
	char t;
	ref = (char *)END_VIDEO;
	t = *ref;
	return;
}

/* paging_test_7
* description: simple test that checks the contents one byte at AFTER_END_VIDEO (0xB9500)
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_7: after end of VIDEO memory */
void paging_test_7()
{
	char* ref;
	char t;
	ref = (char *)AFTER_END_VIDEO;
	t = *ref;
	return;
}

/* paging_test_8
* description: simple test that checks the contents one byte at BEFORE_KERNEL (0x003FFFFF) which is 1 byte before 4MB
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_8: before KERNEL memory */
void paging_test_8()
{
	char* ref;
	char t;
	ref = (char *)BEFORE_KERNEL;
	t = *ref;
	return;
}

/* paging_test_9
* description: simple test that checks the contents one byte at KERNEL (0x00400000) which is at 4MB
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_9: at KERNEL memory */
void paging_test_9()
{
	char* ref;
	char t;
	ref = (char *)KERNEL;
	t = *ref;
	return;
}

/* paging_test_10
* description: simple test that checks the contents one byte after 4MB, AFTER_KERNEL (0x00400001) 
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_10: one after KERNEL memory */
void paging_test_10()
{
	char* ref;
	char t;
	ref = (char *)AFTER_KERNEL;
	t = *ref;
	return;
}

/* paging_test_11
* description: simple test that checks the contents at an address location in 7MB, MID_KERNEL (0x00700000) 
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_11: middle of KERNEL memory */
void paging_test_11()
{
	char* ref;
	char t;
	ref = (char *)MID_KERNEL;
	t = *ref;
	return;
}

/* paging_test_12
* description: simple test that checks the contents at an address location 1 byte above 8MB,  BEFORE_END_KERNEL (0x007FFFFF)
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_12: before end KERNEL memory */
void paging_test_12()
{
	char* ref;
	char t;
	ref = (char *)BEFORE_END_KERNEL;
	t = *ref;
	return;
}

/* paging_test_13
* description: simple test that checks the contents at an address location at 8MB,  END_KERNEL (0x00800000)
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_13: end of KERNEL memory */
void paging_test_13()
{
	char* ref;
	char t;
	ref = (char *)END_KERNEL;
	t = *ref;
	return;
}

/* paging_test_14
* description: simple test that checks the contents at an address location after 8MB,  AFTER_END_KERNEL (0x00850000)
* input: None
* output: None
* returns: 
*	- if test passed, it would appear nothing in kernel
* 	- if test failed, kernel will show page fault exception
side effects: None
*/
/* paging_test_14: end of KERNEL memory */
void paging_test_14()
{
	char* ref;
	char t;
	ref = (char *)AFTER_END_KERNEL;
	t = *ref;
	return;
}

void my_syscall() {
    // Define a variable to hold the system call number
    int syscall_num = 0;

    // Define any necessary arguments for the system call

	int return_val;
    // Use inline assembly to make the system call
    asm volatile("int $0x80"
                 : "=a" (return_val)
                 : "a" (syscall_num)
                 : "memory");
}



/* --------------------------------------------- END OF CHECKPOINT 1 TESTS ---------------------------------------------------------------------------------------*/

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests()
{
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("interrupts", test_interrupts());    
	// launch your tests here
    // divide_by_0_test();
	// my_syscall(); //tests to see if system call was raised in the IDT.
/* --------------------------------------------- START OF CHECKPOINT 1 TESTS CALLS ---------------------------------------------------------------------------------------*/
	//paging_test_0(); // Page Fault at 0
	//paging_test_1(); // Page Fault one before VIDEO
	// paging_test_2(); // NO page fault at VIDEO
	//paging_test_3(); // NO page fault one after VIDEO
	//paging_test_4(); // NO page fault in middle of VIDEO
	//paging_test_5(); // NO page fault one before end of VIDEO
	//paging_test_6(); // Page Fault at end of VIDEO
	//paging_test_7(); // Page Fault after end of VIDEO
	//paging_test_8(); // Page Fault one before KERNEL
	
	//paging_test_9(); // No page fault at KERNEL
	// paging_test_10(); // No page fault one after KERNEL
	//paging_test_11(); // No page fault in middle of KERNEL
	//paging_test_12(); // No page fault one before end of KERNEL
	//paging_test_13(); // Page fault at end of KERNEL
	paging_test_14(); // Page fault after end of KERNEL memory
	

	
/* --------------------------------------------- END OF CHECKPOINT 1 TESTS ---------------------------------------------------------------------------------------*/

} 
