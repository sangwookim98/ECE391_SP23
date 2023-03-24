#include "page.h"
#include "lib.h"

#define VIDEO_MEM 0xB8000

void page_init()
{

    /* Right shift the location of video memory by 12 to make it 20 bits. */
    /* Shift is needed to make sure to match the size of the page address in the struct. */ 
    unsigned int mask_idx = (VIDEO_MEM >> 12);
    int i;

    /* Initialize all 1024 entries in the page_directory table which maps up to 4GB in total */
    for (i = 0; i < 1024; i++)
    {
        /* Sets all the elements of the small_page struct_directory to 0 */
        page_directory[i].small_page.present = 0;
        page_directory[i].small_page.r_w = 0;
        page_directory[i].small_page.u_s = 0;
        page_directory[i].small_page.write_through = 0;
        page_directory[i].small_page.cache_disabled = 0;
        page_directory[i].small_page.accessed = 0;
        page_directory[i].small_page.reserved = 0;
        page_directory[i].small_page.page_size = 0;
        page_directory[i].small_page.global_page = 0;
        page_directory[i].small_page.available = 0;
        page_directory[i].small_page.page_base_addr = 0;
    }

    /* Initializes all 1024 entries in the page table, which maps up to 4 MB in total. */
    for (i = 0; i < 1024; i++)
    {
        /* Initializes the first page table. */
        /* Sets all the elements of the first_page_struct to 0 */
        /* The address is set to i to match the virtual memory to the physical memory. */
        first_page_table[i].present = 0;
        first_page_table[i].r_w = 0;
        first_page_table[i].u_s = 0;
        first_page_table[i].write_through = 0;
        first_page_table[i].cache_disabled = 0;
        first_page_table[i].accessed = 0;
        first_page_table[i].dirty = 0;
        first_page_table[i].page_table_att_idx = 0;
        first_page_table[i].global_page = 0;
        first_page_table[i].available = 0;
        first_page_table[i].page_base_addr = i;
    }

    /* To turn a page_table or page_directory on, we turned on the present and r_w bits. */

    /* Only turns on the video memory page table entry which is indexed at 0xB8. */
    first_page_table[mask_idx].present = 1;
    first_page_table[mask_idx].r_w = 1;

    /* Sets up the first page directory entry that stores the video memory page table entry. */ 
    page_directory[0].small_page.present = 1;
    page_directory[0].small_page.r_w = 1;

    /* Points to the first page table */
    page_directory[0].small_page.page_base_addr = ((uint32_t)first_page_table) >> 12;

    /* Creates the second entry in the page directory table (kernel space) */
    /* Initializes the big page table directory table as the page_size_on bit will now be
     on which directlty maps the 4MB page in the page directory entry to physical memory. */ 
    page_directory[1].big_page.present = 1;
    page_directory[1].big_page.r_w = 1;
    page_directory[1].big_page.u_s = 0;
    page_directory[1].big_page.write_through = 0;
    page_directory[1].big_page.cache_disabled = 0;
    page_directory[1].big_page.accessed = 0;
    page_directory[1].big_page.dirty = 0;
    page_directory[1].big_page.page_size = 1;
    page_directory[1].big_page.global_page = 0;
    page_directory[1].big_page.available = 0;
    page_directory[1].big_page.pat = 0;
    page_directory[1].big_page.reserved = 0;
    
    /* Sets the base address to point to the start of 4 MB in physical memory */
    /* Left shift by 22 because it needs the top 10 bits (the way the bits are set up for a 4MB struct). */
    /* 0x400000 represents the memory location of 4MB. */ 
    page_directory[1].big_page.page_base_addr = (0x400000) >> 22;

    /* Loads page directory and enables paging (call the assembly functions from page_asm.S) */
    loadPageDirectory((uint32_t*)page_directory);
    enablePaging();
}

