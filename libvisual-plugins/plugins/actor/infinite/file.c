#include <stdlib.h>
#include <stdio.h>

#include "renderer.h"

/* hihi */
static uint8_t _inf_shitloadofdata[] = {
0x6, 0x0, 0x0, 0x0, 0x0, 0x1f, 0x1c, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3b,
0x0, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0,
0x0, 0x52, 0x0, 0x0, 0x0, 0x6, 0x0, 0x0, 0x0, 0x80, 0xd5, 0x1b, 0x0, 0x0,
0x0, 0x0, 0x0, 0x3b, 0x0, 0x0, 0x0, 0xb, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 
0x0, 0x1, 0x0, 0x0, 0x0, 0x28, 0x0, 0x0, 0x0, 0x6, 0x0, 0x0, 0x0, 0x40, 
0x95, 0x19, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3b, 0x0, 0x0, 0x0, 0x1d, 0x0, 0x0, 
0x0, 0xff, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6, 0x0, 
0x0, 0x0, 0x80, 0xf3, 0x18, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3b, 0x0, 0x0, 0x0, 
0x4, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x49, 0x0, 
0x0, 0x0, 0x6, 0x0, 0x0, 0x0, 0x80, 0x64, 0x18, 0x0, 0xff, 0x0, 0x0, 0x0, 
0x4b, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 
0x0, 0x71, 0x0, 0x0, 0x0, 0x5, 0x0, 0x0, 0x0, 0x0, 0x96, 0x17, 0x0, 0xff, 
0x0, 0x0, 0x0, 0x3b, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 
0x0, 0x1, 0x0, 0x0, 0x0, 0x62, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 
0x75, 0x16, 0x0, 0xff, 0x0, 0x0, 0x0, 0x52, 0x0, 0x0, 0x0, 0x29, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x8d, 0x1, 0x0, 0x0, 0x4, 0x0,
0x0, 0x0, 0xc0, 0x61, 0x16, 0x0, 0xff, 0x0, 0x0, 0x0, 0x52, 0x0, 0x0, 0x0, 0x29, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x8d, 0x1, 0x0, 0x0, 
0x3, 0x0, 0x0, 0x0, 0x80, 0x73, 0x15, 0x0, 0x0, 0x0, 0x0, 0x0, 0x33, 0x0, 
0x0, 0x0, 0x24, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x80, 0xc1, 0x14, 0x0, 0x0, 0x0,
0x0, 0x0, 0x33, 0x0, 0x0, 0x0, 0xf, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x4, 
0x0, 0x0, 0x0, 0x52, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x40, 0x88, 0x14, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x33, 0x0, 0x0, 0x0, 0x9, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 
0x0, 0x3, 0x0, 0x0, 0x0, 0x3d, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0xc0, 0x4e, 
0x13, 0x0, 0x0, 0x0, 0x0, 0x0, 0x33, 0x0, 0x0, 0x0, 0x9, 0x0, 0x0, 0x0, 
0xff, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x49, 0x0, 0x0, 0x0, 0x3, 0x0, 
0x0, 0x0, 0x0, 0xe8, 0x12, 0x0, 0x0, 0x0, 0x0, 0x0, 0x33, 0x0, 0x0, 0x0, 
0x9, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x4c, 0x0,
0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 0xea, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0,
0x33, 0x0, 0x0, 0x0, 0x9, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x4, 0x0, 
0x0, 0x0, 0x4d, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x40, 0x8d, 0x10, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x33, 0x0, 0x0, 0x0, 0x6, 0x0, 0x0, 0x0, 0xff, 0x0, 
0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x3f, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 
0x40, 0x42, 0xf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x33, 0x0, 0x0, 0x0, 0x6, 0x0, 0x0, 0x0, 
0xff, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x38, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 
0x0, 0x0, 0x6f, 0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x33, 0x0, 0x0, 0x0, 0x6, 0x0,
0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x52, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x40, 0xea, 0xa, 0x0, 0xff, 0x0, 0x0, 0x0, 0x33, 0x0, 0x0, 
0x0, 0x6, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x66, 
0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x40, 0xc6, 0xa, 0x0, 0xff, 0x0, 0x0, 0x0, 
0x33, 0x0, 0x0, 0x0, 0x6, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 
0x0, 0x66, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x8, 0xa, 0x0, 0xff, 0x0, 
0x0, 0x0, 0x3a, 0x0, 0x0, 0x0, 0x1d, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 
0x0, 0x0, 0x0, 0x8d, 0x1, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0xc0, 0x1, 0xa, 0x0, 
0xff, 0x0, 0x0, 0x0, 0x3a, 0x0, 0x0, 0x0, 0x1d, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x4, 0x0, 0x0, 0x0, 0x8d, 0x1, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x40, 0x58, 
0x9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x54, 0x0, 0x0, 0x0, 0x1d, 0x0, 0x0, 0x0, 0xff, 
0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 
0xa4, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x54, 0x0, 0x0, 0x0, 0x17, 0x0, 0x0, 0x0, 
0xff, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 
0x80, 0xe2, 0x7, 0x0, 0xff, 0x0, 0x0, 0x0, 0x54, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 
0x0, 0xff, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 
0x0, 0xc0, 0x8e, 0x7, 0x0, 0xff, 0x0, 0x0, 0x0, 0x54, 0x0, 0x0, 0x0, 0xa, 0x0, 
0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x40, 0xbb, 0x5, 0x0, 0xff, 0x0, 0x0, 0x0, 0x54, 0x0, 0x0, 0x0,
0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x2d, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x89, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0xb, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x3e, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc0, 0xdf, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0xb, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x4c, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc0, 0x4f, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x2b, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0 };

t_effect _inf_effects[100];
int _inf_nb_effects=0;
                                                                                                                                               
void _inf_load_effects(InfinitePrivate *priv)
{
	int i;
	int index = 0;

	while (1) {
		uint8_t* ptr_effect= (uint8_t*) &_inf_effects[_inf_nb_effects];

		for (i = 0; i < sizeof (t_effect); i++) {
			int b = _inf_shitloadofdata[index++];

			if (_inf_nb_effects >= 29) {
				_inf_nb_effects -= 1;

				return;
			}

			ptr_effect[i]=(uint8_t)b;
		}

		_inf_nb_effects+=1;
	}
}

void _inf_load_random_effect(InfinitePrivate *priv, t_effect *effect)
{
	if (_inf_nb_effects > 0) {
		int num_effect=rand()%_inf_nb_effects;
		int i;
		
		for (i = 0 ; i < sizeof (t_effect); i++) {
			*((uint8_t*)effect+i)=*((uint8_t*)(&_inf_effects[num_effect])+i);
		}
	}
}
