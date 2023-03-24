#define ASM 1
/*Functions were refrenced from OSDEV*/ 
.text
.globl loadPageDirectory
loadPageDirectory:
pushl %ebp
movl %esp, %ebp
movl 8(%esp), %eax
movl %eax, %cr3
movl %cr4, %eax
orl $0x10, %eax
movl %eax, %cr4
movl %ebp, %esp
popl %ebp
ret

.globl enablePaging
enablePaging:
pushl %ebp
movl %esp, %ebp
movl %cr0, %eax
orl $0x80000001, %eax
movl %eax, %cr0
movl %ebp, %esp
popl %ebp
ret
