#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <fcntl.h>
#include <time.h>
#include "map.h"

#define BASE 0xe010
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
void randomspeed();
void checkspeed();
void setcountdown();
void setcountdownrand();

/* All of these are [Flower#][Petal#][Motor#] */
int phase[NUMENTRIES];
int speed[NUMENTRIES];
int newspeed[NUMENTRIES];
int countdown[NUMENTRIES];
int direction[NUMENTRIES];

unsigned char halfstep[] = { 0x2, 0xA, 0x8, 0x9, 0x1, 0x5, 0x4, 0x6 };
int ref_motor = 0;
int ripple_speed(int);

extern int rsquared[MAXMOTORS];

int wave_radii = 0,wave_width = 350,wave_increment = 350;
int time_to_exit;

main(int argc, char **argv)
{
    int ret;
    int i, loopcount, newloopcount;
    int m;
    unsigned char data;
    int stop;
    int motors;
    int popper_start = 100;
    int popper_fade = 20;
    int time_to_live = 10;

    argc--;*argv++;
    if(argc-- > 0) popper_start = atoi(*argv++);
    if(argc-- > 0) popper_fade = atoi(*argv++);
    if(argc-- > 0) time_to_live = atoi(*argv++);

    printf("popper start = %d popper fade = %d\n",popper_start,popper_fade);
    srand(getpid());

    compute_radii(rand()%343);
    popper_speed(rand()%5*wave_width);

    /* 
     * Set STDIN non-blocking so we can use getchar() to see if a CR
     * was entered, and reset to position = 0
     */
    ret = fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NDELAY);

    /*
     * We need to access the parallel port as root.
     */
    ret = iopl( 3 );
    if (ret < 0) {
	printf("iopl returned %d\n",ret);
	printf("try running as root\n");
	exit(1);
    }
    ret = ioperm(BASE, 5, 1);
    if (ret < 0) {
	printf("Could not get permission to access the parallel port\n");
	printf("try running as root\n");
	exit(1);
    }

    stop = 0;

    for (m = 0; m < NUMENTRIES; m++) {
	speed[m] = 1000;
	phase[m] = 1;
    }
#if(0)
    randomspeed();
    setcountdownrand();
#endif

    time_to_exit = (clock()/CLOCKS_PER_SEC) + time_to_live;
    loopcount = newloopcount = 0;
    while (stop != 1) {
	if (getchar() != EOF)
	    stop = 1;
	for (m = 0; m < NUMENTRIES; m++) {
	    if (countdown[m] == 0) {
		countdown[m] = speed[m];	/* Reset counter */
		if(direction[m] == 0) {
			phase[m]++;
			phase[m] %= MAXPHASE;
		} else {
			phase[m]--;
			if(phase[m] < 0) phase[m] = MAXPHASE-1;
		} 
	    }
	    countdown[m]--;
	}
	update_drivers();
	if (loopcount++ > popper_start) {
	    loopcount = 0;
    	    compute_radii(rand()%343);
    	    popper_speed(rand()%5*wave_width);
	    setcountdown();
            checklive();
	}
	if (newloopcount++ > popper_fade) {
	    newloopcount = 0;
	    checkspeed();
	    setcountdown();
	}
	/* usleep(1); */
    }
}

void sendbyte(unsigned char data)
{
    outb(data, BASE);
    outb(1, BASE+2);
    delay(100);
    outb(9, BASE+2);
}

void sendbit(unsigned char data)
{
    outb((data & 1), BASE);
    outb(1, BASE+2);
    delay(100);
    outb(9, BASE+2);
}

void sendstrobe()
{
    outb(13, BASE+2);
    delay(100);
    outb(9, BASE+2);
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

void randomspeed()
{
    int m;

#if (1)
    for (m = 0; m < NUMENTRIES; m++) {
	newspeed[m] = rand() % 80;
	direction[m] = rand()%2;
    }
#endif
#if(0)
    for (m = 0; m < 2; m++)
	newspeed[rand()%NUMENTRIES] = (rand() % 100) + 1;
#endif
}

void
checkspeed()
{
    int m;

    for (m = 0; m < NUMENTRIES; m++) {
	if (newspeed[m] < speed[m])
	    speed[m]--;
	else if (newspeed[m] > speed[m])
	    speed[m]++;
    }
}

void
setcountdown()
{
    int m;

    for (m = 0; m < NUMENTRIES; m++)
	countdown[m] = speed[m];
}
void
setcountdownrand()
{
    int m;

    for (m = 0; m < NUMENTRIES; m++)
	countdown[m] = speed[m]+rand()%50;
}

delay(int delay_value) {
    int i,a,b,c;
    a = 0;
    b = 1;
    c = 6;

    for(i=0;i<delay_value;i++) a+= b * c;
}


int
popper_speed(int wave_radii) {
    int i;
    int motors = 0;

    for(i=0;i<MAXMOTORS;i++) {
        if(rsquared[i] < wave_radii) {
	    newspeed[map[i]] = 2;
	    speed[map[i]] = 2;
	    motors++;
	}
        else newspeed[map[i]] = 1000;
    }
    return motors;
}

checklive() {
    if((clock()/CLOCKS_PER_SEC) > time_to_exit) exit(0);
}

