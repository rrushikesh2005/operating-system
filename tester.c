#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

int main () {
    void *ptr = mmap(0,4096*4096,PROT_READ | PROT_WRITE, MAP_PRIVATE+MAP_ANONYMOUS,-1,0);
    // for (int i=0; i<100; i++) {
    //     sprintf(ptr,"a");
    //     ptr++;
    // }
    sleep(50);
}