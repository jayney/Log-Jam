/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2014 Karl Palsson <karlp@tweak.net.au>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include "responsive_adc.h"

#include "defines.h"
#include "init.h"
#include "globals.h"
#include "adsr.h"



uint8_t		adsr_status[2];
bool			key_status[2];





int main(void)
{


	init_hw();
	adsr_init(0, 0.001, 0.002, 0.8,0.0005,SHAPE_POW2);
	adsr_init(1, 0.002, 0.001, 0.6,0.001, SHAPE_POW3);	

while(1)
  {
		adsr_status[0] = get_adsr_status(0);			// Findout what state our adsr is in.
		key_status[0] = gpio_get(GPIOB, GATE1);		// Key up = True, Key down = false...
		adsr_status[1] = get_adsr_status(1);
		key_status[1] = gpio_get(GPIOB, GATE2);

	// If the key is down, AND envelope isn't already running, start it
		if((!key_status[0]) && (adsr_status[0] == PHASE_STOPPED))
		{
			adsr_start(0);	
		}	

		if((!key_status[1]) && (adsr_status[1] == PHASE_STOPPED))		
		{
			adsr_start(1);			
		}	

	// Now handle what happens if the key is down but envelope is in the release phase
	// Need to allow for where the output is at that point

		if((!key_status[0]) && (adsr_status[0] == PHASE_RELEASE))		
		{
			adsr_restart(0);
		}

		if((!key_status[1]) && (adsr_status[1] == PHASE_RELEASE))			
		{
			adsr_restart(1);
		}

	// And finally handle the release if the key is up, and the adsr is running but not already in release.
		if((key_status[0]) && (adsr_status[0] > PHASE_STOPPED) && (adsr_status[0] < PHASE_RELEASE))			
		{
			adsr_release(0);
		}

		if((key_status[1]) && (adsr_status[1] > PHASE_STOPPED) && (adsr_status[1] < PHASE_RELEASE))			
		{
			adsr_release(1);
		}
  }	

	return 0;
}