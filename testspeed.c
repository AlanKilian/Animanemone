#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "unistd.h"
#include <fcntl.h>
#include <pi-gpio.h>

#define BASE 0x378
#define NUMFLOWERS 7
#define NUMPETALS 7
#define NUMMOTORS 8
#define NUMENTRIES (NUMPETALS*NUMMOTORS*NUMFLOWERS)

/* 8 "phases" when half-stepping */
#define MAXPHASE 8
#define MAXMOTORS 343

void sendbyte(unsigned char data);
void sendbit(unsigned char data);
void sendstrobe();
void update_drivers();

/* All of these are [Flower#][Petal#][Motor#] */
int phase[NUMENTRIES];
int speed[NUMENTRIES];
int newspeed[NUMENTRIES];
int countdown[NUMENTRIES];
int direction[NUMENTRIES];

unsigned char halfstep[] = { 0x2, 0xA, 0x8, 0x9, 0x1, 0x5, 0x4, 0x6 };

void delay(int);

int
main(int argc, char **argv)
{
    int ret;
    int m;
    unsigned char data;
    int stop;
    int motors;

    setup(); //start pi-gpio stuff
    setup_gpio(18, OUTPUT, 0); // Set for latch
    setup_gpio(15, OUTPUT, 0); // Set for clock
    
    setup_gpio(21, OUTPUT, 0); // Set for D0
    setup_gpio(20, OUTPUT, 0); // Set for D1
    setup_gpio(16, OUTPUT, 0); // Set for D2
    setup_gpio(12, OUTPUT, 0); // Set for D3
    setup_gpio( 7, OUTPUT, 0); // Set for D4
    setup_gpio( 8, OUTPUT, 0); // Set for D5
    setup_gpio(25, OUTPUT, 0); // Set for D7

    for (m = 0; m < NUMENTRIES; m++) {
    speed[m] = 2;
    phase[m] = 1;
    }
    while(1) {
        update_drivers();
        for (m = 0; m < NUMENTRIES; m++) {
            phase[m]++;
            phase[m] %= MAXPHASE;
        }
    }
}

void sendbyte(unsigned char data)
{
    output_gpio(21, data && 0x01);    // Set d0 depending on the corresponding bit in data
    output_gpio(20, data && 0x02);    // Set d1 depending on the corresponding bit in data
    output_gpio(16, data && 0x04);    // Set d2 depending on the corresponding bit in data
    output_gpio(12, data && 0x08);    // Set d3 depending on the corresponding bit in data
    output_gpio( 7, data && 0x10);    // Set d4 depending on the corresponding bit in data
    output_gpio( 8, data && 0x20);    // Set d5 depending on the corresponding bit in data
    output_gpio(25, data && 0x40);    // Set d6 depending on the corresponding bit in data
    
    output_gpio(15, 1);    // Set clock signal high
    delay(300);            // This is 1.5 microseconds wide
    output_gpio(15, 0);    // set clock signal low
    delay(1200);        // Delay so that the latch-signal-per-second come out right
}

void sendstrobe()
{
    output_gpio(18, 1);    // Set latch signal high
    delay(300);            // This is 1.5 microseconds wide
    output_gpio(18, 0);    // Set latch signal low
}

void update_drivers()
{
    int f,p,m,i;
    unsigned char data[NUMFLOWERS];
    unsigned char eightbits;

    for(p=0;p<NUMPETALS;p++) {
        for(m=0;m<NUMMOTORS;m++) {
            for(f=0;f<NUMFLOWERS;f++) {
                data[f] = halfstep[phase[(f*(NUMPETALS*NUMMOTORS)+(p*NUMMOTORS))+m]];
            }
            for(i=0;i<4;i++) {
                eightbits = 0;
                    for(f=NUMFLOWERS-1;f>=0;f--) {
                        eightbits <<= 1;
                        eightbits |= data[f] & 1;
                        data[f] >>= 1;
                    }
                sendbyte(eightbits);
            }
        }
    }
    sendstrobe();
}

void
delay(int delay_value) {
    int i,a,b,c;
    a = 0;
    b = 1;
    c = 6;

    for(i=0;i<delay_value;i++) a+= b * c;
}
