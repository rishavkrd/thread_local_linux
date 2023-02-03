#define SYS_copy_pte 326
#define SYS_change_pte_prot 327
#define SYS_zc_send 328
#define SYS_zc_recv 329

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/mman.h>

// #define PROT_READ	0x1		/* page can be read */
// #define PROT_WRITE	0x2		/* page can be written */

// #define MAP_SHARED	0x01		/* Share changes */
// #define MAP_PRIVATE	0x02		 //Changes are private 
// #define MAP_TYPE	0x0f		/* Mask for type of mapping (OSF/1 is _wrong_) */
// #define MAP_FIXED	0x100		/* Interpret addr exactly */
// #define MAP_ANON	0x10		/* don't use a file */

typedef struct { unsigned long pte; } pte_t;
int ret=10;




int zc_send(const char *buffer, size_t size)

{

    asm volatile (

        "syscall"                  // Use syscall instruction

        : "=a" (ret)               // Return value in RAX

        : "0"(SYS_zc_send),        // Syscall numer

          "D"(buffer),             // 1st parameter in RDI (D)

          "S"(size)                // 2nd parameter in RSI (S)

        : "rcx", "r11", "memory"); // clobber list (RCX and R11)

    return ret;

}


int zc_recv(char *buffer, size_t size)

{

    asm volatile (

        "syscall"                  // Use syscall instruction

        : "=a" (ret)               // Return value in RAX

        : "0"(SYS_zc_recv),        // Syscall numer

          "D"(buffer),             // 1st parameter in RDI (D)

          "S"(size)                // 2nd parameter in RSI (S)

        : "rcx", "r11", "memory"); // clobber list (RCX and R11)

    return ret;

}

int copy_pte(unsigned long from_va, unsigned long to_va, pte_t * pte)
{

    asm volatile (

        "syscall"                  // Use syscall instruction

        : "=a" (ret)               // Return value in RAX

        : "0"(SYS_copy_pte),       // Syscall numer

          "D"(from_va),            // 1st parameter in RDI (D)

          "S"(to_va),              // 2nd parameter in RSI (S)

          "d"(pte)                 // 3rd parameter in RDX (d)

        : "rcx", "r11", "memory"); // clobber list (RCX and R11)

    return ret;

}




int change_pte_prot(unsigned long va, int prot, int flushtlb)

{

    asm volatile (

        "syscall"                  // Use syscall instruction

        : "=a" (ret)               // Return value in RAX

        : "0"(SYS_change_pte_prot),       // Syscall numer

          "D"(va),                 // 1st parameter in RDI (D)

          "S"(prot),               // 2nd parameter in RSI (S)

          "d"(flushtlb)            // 3rd parameter in RDX (d)

        : "rcx", "r11", "memory"); // clobber list (RCX and R11)

    return ret;

}

int main(int argc, const char ** argv){
	//printf("output: %d \n",copy_pte(0,0,0));
    char sval[] = "Hello World";
    char rval[20];

    zc_send(sval,sizeof(sval));
    zc_recv(rval,sizeof(sval));
    printf("sval size: %d, rval size: %d \n",sizeof(sval),sizeof(rval));

    printf("received value: %s\n",rval);

    return 0;
	// int *addr1;

 //    int *addr2;

 //    int ret=0;
 //    int change=10;

 //    pte_t pte;


 //    // addr1 = (int *) mmap ( NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );
 //    // addr2 = (int *) mmap ( NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );

 //    addr1 = (int *) mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
 //    addr2 = (int *) mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);

 //    //int *ptr = mmap ( NULL, N*sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );

 //    printf("addr1,2 initialized \n");
 //    if (!addr1 || !addr2) {

 //        printf("mmap() failed \n");

 //        return -1;

 //    }


 //     //Access both addresses to make sure they are populated. 

 //    *addr1 = 0;

 //    *addr2 = 1;
 //    printf("addr1,2 alloted \n");

 //    //ret = copy_pte(addr1, addr2, &pte);

 //    if (ret < 0) {

 //        printf("copy_pte() failed \n");

 //        return -1;
 //    }
 //    if (*addr1 == *addr2)

 //        printf("addr1 and addr2 map to the same page\n");

    
 //    // change= change_pte_prot(0,0,0);
 //    // printf("task 2: change_pte_prot test returns: %d \n",change);

 //    volatile int *addr = (volatile int *) mmap(NULL, 4096, PROT_READ, MAP_ANON|MAP_PRIVATE, -1, 0);


 //    if (!addr) {

 //        printf("mmap() failed");

 //        return -1;

 //    }
 //    printf("trying to Access addr\n");

 //    /* Access (read) the address to make sure the page is populated. */

 //    *addr;
 //    printf("Accessesed addr successfully\n");

 //    /* Change the PTE to readable and writable, and then flush TLB */

 //    ret = change_pte_prot(addr, PROT_READ|PROT_WRITE, 1);

 //    if (ret < 0) {

 //        printf("change_pte_prot() failed\n");

 //        return -1;

 //    }

 //    printf("trying to write 1 in addr\n");
 //    *addr = 1;

 //    printf("The 1st write is successful!\n");


 //    /* Change the PTE to readonly, and but do not flush TLB */

 //    ret = change_pte_prot(addr, PROT_READ, 0);

 //    if (ret < 0) {

 //        printf("change_pte_prot() failed\n");

 //        return -1;

 //    }


 //    *addr = 2;
 //   printf("The 2nd write is successful!\n");


 //    sleep(3);
 //   *addr = 3;

 //    printf("The 3rd write is successful!\n");

	return 0;
}

