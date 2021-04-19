//
// Simple GPIO memory-mapped example by YoungJin Suh (http://valentis.pe.kr / valentis@chollian.net)
//                           originally from Snarky (github.com/jwatte)
// build with:
//  g++ -O1 -g -o switch switch.cpp -Wall -std=gnu++17
// run with:
//  sudo ./switch
//

#include <stdio.h>
#include <stdlib.h>                     // for exit()
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

#include "gpionano.h"
#include <string>
#include <iostream>

int main(int argc, char** argv)
{
    //  read physical memory (needs root)
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        fprintf(stderr, "usage : $ sudo %s (with root privilege)\n", argv[0]);
        exit(1);
    }

    //  map a particular physical address into our address space
    int pagesize = getpagesize();
    int pagemask = pagesize-1;

    //  This page will actually contain all the GPIO controllers, because they are co-located
    void* base = mmap(0, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, ((GPIO_106) & ~pagemask));
    void* pinmuxbase = mmap(0, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, ((0x70000000 + 0x306C) & ~pagemask));
    
    //volatile uint32_t* pinmux = (volatile uint32_t*)mmap(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x70000000 + 0x306C);

    //  set up a pointer for convenient access -- this pointer is to the selected GPIO controller
    gpio_t volatile *pinLed = (gpio_t volatile *)((char *)base + (GPIO_106 & pagemask));
    volatile uint32_t *pinmux = (volatile uint32_t*)((char*)pinmuxbase + ((0x70000000 + 0x306C)& pagemask)); //ends up adding 0x6C to the address
    //std::cout<<"pinmux register: "<<*((volatile uint32_t*)((uint32_t*)pinmuxbase + ((0x70000000 + 0x306C)& pagemask)))<< std::endl;
    //gpio_t volatile *pinSwt = (gpio_t volatile *)((char *)base + (GPIO_77 & pagemask));
    std::cout<<"*(pinmux) before tristate cleared: "<< *(pinmux)<<std::endl;
	
    //std::cout<<"*(pinmux): "<< *(pinmux)<<std::endl;
    //*((volatile uint32_t*)((uint32_t*)pinmuxbase + ((0x70000000 + 0x3000)& pagemask)+0x6C/4)) &= ~(((1 << 1) - 1) << 4);
    //*((volatile uint32_t*)((uint32_t*)pinmuxbase + ((0x70000000 + 0x3000)& pagemask)+0x6C/4)) |= (1 << 4);
    
    *(pinmux) &= ~(((1 << 1) - 1) << 4);
    std::cout<<"*(pinmux) after tristate clear: "<< *(pinmux)<<std::endl;
    
    // for LED : GPIO OUT 
    pinLed->CNF = 0x00FF;
    pinLed->OE = OUTPUT;
//  pinLed->OUT = 0xFF;
    
    //*(pinmux) = 0x0;
    // for LED : GPIO OUT 
    //pinLed->CNF = 1<<6;
    //pinLed->OE = 1<<6;

    
    // for Switch : GPIO IN 
    //pinSwt->CNF = 0x00FF;
    //pinSwt->OE = INPUT;
    //pinSwt->IN = 0x00; 			// initial value
    
    //  disable interrupts
    //pinLed->INT_ENB = 0x00;
    //pinSwt->INT_ENB = 0x00;

    // parameter for Input
    //pinSwt->INT_STA = 0xFF;		// for Active_low
    //pinSwt->INT_LVL = GPIO_INT_LVL_EDGE_BOTH;
    //pinSwt->INT_CLR = 0xffffff;

    pinLed->OUT = 0xFF;
    //pinLed->OUT = 1<<6;
    //std::cout<<"pinLed: "<< pinLed->OUT<<std::endl;
    sleep(5);    
    // turn led light with switch 
    //printf("checkout : \"$ sudo cat /sys/kernel/debug/tegra_gpio\"\n");
    /*
    for(uint8_t cnt = 0; cnt < 100; cnt++) {
        //printf("[%d] %x\n", cnt, pinSwt->IN>>5);		// for debug message
        printf((pinSwt->IN>>5)?"x":"o");
        pinLed->OUT = (pinSwt->IN>>5)?0:0xff;
	fflush(stdout);
        usleep(50*1000);		// wait 50 milisec
    }
    */

    /* turn off the LED */
    pinLed->OUT = 0x00;
    *(pinmux) |= (1 << 4);
    std::cout<<"*(pinmux) after set: "<< *(pinmux)<<std::endl;
 
    //*((volatile uint32_t*)((uint32_t*)pinmuxbase + ((0x70000000 + 0x3000)& pagemask)+0x6C/4)) &= ~(((1 << 1) - 1) << 4);
    /* unmap */
    munmap(base, pagesize);
    munmap(pinmuxbase, pagesize);
    
    //munmap((void*)pinmux, pagesize);
    /* close the /dev/mem */
    close(fd);

    //printf("\nGood Bye!!!\n");
    
    return 0;
}
