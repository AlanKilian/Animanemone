#include <stdio.h>
#include <math.h>
#include "place.h"

#define X 0
#define Y 1
#define MAXMOTORS 343

int rsquared[MAXMOTORS];
int speed[MAXMOTORS];

void compute_radii(int);
void print_radii();
void print_values();
void print_speeds();

extern int ref_motor;

#if(0)
main (int argc, char **argv) {
    int wave_radii,wave_width = 120000,wave_increment = 120000;
    int i;

    /*
     * Junk stuff
     */
    int s,going;

    argc--;*argv++;
    if(argc-- > 0) ref_motor = atoi(*argv++);
    if(argc-- > 0) wave_width = atoi(*argv++);
    if(argc-- > 0) wave_increment = atoi(*argv++);

    printf("Ref motor = %d, wave_width = %d, wave_increment = %d\n",
	ref_motor,wave_width,wave_increment);

    compute_radii(ref_motor);

    s = 1000;
    going = 1;
    wave_radii = 0;
    while (going < 10){
	for(i=0;i<MAXMOTORS;i++) {
	    if(abs(wave_radii-rsquared[i]) < wave_width) speed[i] = s;
	    else speed[i] = 0;
	}
	going++;
#if(0)
	print_speeds();
#endif
	wave_radii += wave_increment;
    }
}
#endif

void
compute_radii(int ref_motor){
    int i,dx,dy;
    int ref_x,ref_y;

    ref_x = place[ref_motor][X];
    ref_y = place[ref_motor][Y];

    for(i=0;i<MAXMOTORS;i++) {
	dx=place[i][X]-ref_x;
	dy=place[i][Y]-ref_y;
	rsquared[i] = sqrt((dx*dx)+(dy*dy));
    }
}

void
print_radii(){
    int i;
    int ref_x,ref_y;

    printf("Reference motor %d is at %d,%d\n",ref_motor
	                                     ,place[ref_motor][X]
					     ,place[ref_motor][Y]);
    for(i=0;i<MAXMOTORS;i++) {
	printf("Motor %d is %d units away\n",i,rsquared[i]);
    }
}

#if(0)
void
print_values () {
    int i,x,y,channel;

    for(i=0;i<MAXMOTORS;i++) {
	x = place[i][X];
	y = place[i][Y];
	channel = map[i];
	printf("Motor %d is channel %d at %d,%d\n",i,channel,x,y);
    }
}
#endif

void
print_speeds () {
    int i;

    for(i=0;i<MAXMOTORS;i++) {
	printf("Motor %d speed %d\n",i,speed[i]);
    }
}
