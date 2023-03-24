#ifndef ___PAGE_H            /* include for linking */
#ifndef ASM                 /* include this too */
#include "types.h"

extern void page_init();

/* using struct to manually change each value of the members in the struct */

/* might need to change this to normal struct. */
// typedef struct page_directory_entry{
//     struct{
//         uint32_t present : 1;
//         uint32_t r_w : 1;
//         uint32_t u_s : 1;
//         uint32_t write_through : 1;
//         uint32_t cache_disabled : 1;
//         uint32_t accessed : 1;
//         uint32_t reserved : 1;
//         uint32_t page_size : 1;
//         uint32_t global_page: 1;
//         uint32_t available: 3;
//         uint32_t page_base_addr: 20;
//     } __attribute__ ((packed));
// } page_directory_entry_t; 

typedef union page_directory_entry_t{
    
    struct __attribute__((packed)){    
        uint32_t present : 1;
        uint32_t r_w : 1;
        uint32_t u_s : 1;
        uint32_t write_through : 1;
        uint32_t cache_disabled : 1;
        uint32_t accessed : 1;
        uint32_t reserved : 1;
        uint32_t page_size : 1;
        uint32_t global_page: 1;
        uint32_t available: 3;
        uint32_t page_base_addr: 20;
    }small_page;

    struct __attribute__((packed)){
        uint32_t present : 1;
        uint32_t r_w : 1;
        uint32_t u_s : 1;
        uint32_t write_through : 1;
        uint32_t cache_disabled : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t page_size : 1;
        uint32_t global_page: 1;
        uint32_t available: 3;
        uint32_t pat: 1;
        uint32_t reserved: 9;
        uint32_t page_base_addr: 10; 
    }big_page;

} page_directory_entry_t; 



// typedef struct page_table_entry{
//     uint32_t val;
//     struct{
//         uint32_t present : 1;
//         uint32_t r_w : 1;
//         uint32_t u_s : 1;
//         uint32_t write_through : 1;
//         uint32_t cache_disable : 1;
//         uint32_t accessed : 1;
//         uint32_t dirty : 1;
//         uint32_t page_table_att_idx : 1;
//         uint32_t global_page: 1;
//         uint32_t available: 3;
//         uint32_t page_base_addr: 20;
//     } __attribute__ ((packed));
// } page_table_entry_t;

typedef struct __attribute__ ((packed)){
    uint32_t present : 1;
    uint32_t r_w : 1;
    uint32_t u_s : 1;
    uint32_t write_through : 1;
    uint32_t cache_disabled : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t page_table_att_idx : 1;
    uint32_t global_page: 1;
    uint32_t available: 3;
    uint32_t page_base_addr: 20;
} page_table_entry_t;



// Page Direcctory Entry
page_directory_entry_t page_directory[1024] __attribute__((aligned(4096)));

// Page Table 
page_table_entry_t first_page_table[1024] __attribute__((aligned(4096)));


extern void loadPageDirectory(uint32_t* addr);
extern void enablePaging();

#endif
#endif

