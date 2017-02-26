#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

typedef bool boolean;

#include "../../FirstGame/mazegen.h"

int main(int argc, char *argv[]) {

	uint8_t x,y;
	bool done=false;
	uint32_t seed;
	time_t t;

        if(argc < 2) {
                seed = time(&t);
        } else {
                seed = atoi(argv[1]);
        }

        printf("Seed: %d\n\n", seed);

	resetGen(seed);

	while(!done){
		printf("Generating...\n");
		done = genMap();
	}


	for(y=0;y<MAPH;y++) {
		for(x=0;x<MAPW;x++) {
			if(get_map(x, y, map_0)) {
				putchar('[');putchar(']');
			} else {
					putchar(' ');putchar(' ');
			}
		}
		putchar('\n');

	};

	return 0;
}
