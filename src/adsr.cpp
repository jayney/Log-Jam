#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/timer.h>

#include <libopencm3/stm32/dac.h>
//#include <libopencm3/stm32/gpio.h>


// ADSR status definitions
#define PHASE_STOPPED       0
#define PHASE_ATTACK        1
#define PHASE_ATTACK_DONE   2
#define PHASE_DECAY         3
#define PHASE_DECAY_DONE    4
#define PHASE_SUSTAIN       5
#define PHASE_RELEASE       6

// ADSR Directions
#define ENV_UP              1
#define ENV_DOWN            0

//ADSR Shape - Note, LIN only applies to attack, and decay/release default to POW2
#define SHAPE_LIN           0
#define SHAPE_POW2          1
#define SHAPE_POW3          2
#define SHAPE_POW4          3

struct Envelope
  {
    float    		attack;
    float    		decay;
    float    		sustain;
    float    		release;

    uint8_t     phase;
    uint8_t     direction;
    uint8_t     shape;

    float				min;
    float    		max;
    float       position;
    float       step;
    float       scale;   
    float       point;
    float   		 v_out;
  };

Envelope      env[2];                         // It's a dual ADSR Module, so make an array of 2
uint8_t				envgen =0;

/*
#define T7_PRESCALER        20
#define T7_PERIOD           350
static void timer_setup(void)
{
// All of these are pretty self explanatory.
//Basically sets up a basic timer that will generate an interrupts
//at the sample rate.
	rcc_periph_clock_enable(RCC_TIM7);
	nvic_enable_irq(NVIC_TIM7_IRQ);
	rcc_periph_reset_pulse(RST_TIM7);

	timer_set_prescaler(TIM7, T7_PRESCALER);
	timer_set_period(TIM7, T7_PERIOD);
	timer_enable_preload(TIM7);
	timer_continuous_mode(TIM7);
	timer_enable_counter(TIM7);
	timer_enable_irq(TIM7, TIM_DIER_UIE);
}
*/














/*---------------------------------------------------------------------------*/
/** @brief Get the current status(phase) of an envelope generator

@param[in] egen uint8_t  Envelope Generator to get the value from 
@returns uint8_t - Currently running phase of the specified envelope generator
*/
uint8_t get_adsr_status(uint8_t egen)
{
  return env[egen].phase;
}

/*---------------------------------------------------------------------------*/
/** @brief Set initial parameters of an envelope generator
@param[in] egen uint8_t  Envelope Generator to set a parameter to
@param[in] a_val float  Attack value 0..1
@param[in] d_val float  Decay value 0..1 
@param[in] s_val float  Sustain value 0..1 
@param[in] r_val float  Release value 0..1  
@param[in] shape uint8_t  Specifies math fucntion to use for envelope shape  
*/
void adsr_init(uint8_t egen, float a_val, float d_val, float s_val, float r_val, uint8_t shape)
{
	env[egen].phase = PHASE_STOPPED;	
	env[egen].position = 0;
	if(a_val >=0 && a_val <= 1)	env[egen].attack = a_val;
	if(d_val >=0 && d_val <= 1)	env[egen].decay = d_val;
	if(s_val >=0 && s_val <= 1)	env[egen].sustain = s_val;
	if(r_val >=0 && r_val <= 1)	env[egen].release = r_val;
	if(shape >=0 && shape <= 3) env[egen].shape = shape;
}


/*---------------------------------------------------------------------------*/
/** @brief Set an individual parameter of an envelope generator
@param[in] egen uint8_t  Envelope Generator to set a parameter to
@param[in] mode uint8_t  Parameter to change  
@param[in] value float  New parameter value 
*/
void adsr_set(uint8_t egen, uint8_t mode, float value)
{
	switch(mode)
	{
		case PHASE_ATTACK:
			if(value >=0 && value <= 1)	env[egen].attack = value;
			break;

		case PHASE_DECAY:
			if(value >=0 && value <= 1) env[egen].decay = value;
			break;

		case PHASE_SUSTAIN:
			if(value >=0 && value <= 1) env[egen].sustain = value;
			break;

		case PHASE_RELEASE:
			if(value >=0 && value <= 1) env[egen].release = value;
			break;

		default:
		  break;
	}
}

/*---------------------------------------------------------------------------*/
/** @brief Triggers the start of an envelope

@param[in] egen uint8_t  Envelope Generator to start 
*/
void adsr_start(uint8_t egen)
{
			env[egen].position = 0;
			env[egen].phase = PHASE_ATTACK;
}


/*---------------------------------------------------------------------------*/
/** @brief Restarts an envelope if it's currently in release phase
so it takes into account the current v_out and starts from there

@param[in] egen uint8_t  Envelope Generator to restart 
*/
void adsr_restart(uint8_t egen)
{
			env[egen].position = 0;
			env[egen].min = env[egen].v_out; env[egen].max = 1;
			env[egen].phase = PHASE_ATTACK;
}

/*---------------------------------------------------------------------------*/
/** @brief Triggers the release phase of an envelope

@param[in] egen uint8_t  Envelope Generator to release 
*/

void adsr_release(uint8_t egen)
{
			env[egen].position = 0;
			env[egen].min = 0; env[egen].max = env[egen].v_out;
			env[egen].phase = PHASE_RELEASE;
}




/*---------------------------------------------------------------------------*/
/** @brief Called by ISR to calculate the next point on the curve

not done very well... Needs a revamp with params and a return
 (but that might slow it down. Maybe if I port to H7)
 and this works. If it ain't broke....
*/
void curve_point(void)
{
	float t1,t2,t3;
	env[envgen].scale = ((float)env[envgen].max - (float)env[envgen].min);
	t1 = 1- env[envgen].position;
	t2 = t1*t1;
	switch (env[envgen].shape)
	{
		case	SHAPE_LIN:	
			if (env[envgen].phase == PHASE_ATTACK) t3 = t1;
			else t3 = t2 * t2;
			break;

		case SHAPE_POW2:
			t3 = t2;
			break;

		case SHAPE_POW3:
			t3 = t1 * t2;
			break;

		case SHAPE_POW4:
		  t3 = t2 * t2;
			break;
	}
	if(env[envgen].direction) env[envgen].point = 1-t3;
	else env[envgen].point = t3;
	env[envgen].v_out = env[envgen].min + (env[envgen].scale * env[envgen].point);	
}


/*---------------------------------------------------------------------------*/
/** @brief ISR for Timer 7 to generate the next set of envelope points
*/
void tim7_isr(void)
{
  //GPIO_BSRR(GPIOA) = GPIO9;       //Set GPIOA Pin 9 For Time Measurement. Direct HW because it's faster :-)
  TIM_SR(TIM7) = ~TIM_SR_UIF;     // Clear TIM7 Interrupt Flag


for(envgen = 0; envgen<2; envgen++)
{
	switch(env[envgen].phase)
	{
		case PHASE_ATTACK:
			env[envgen].max = 1; env[envgen].direction = ENV_UP;			
			curve_point();
			env[envgen].position += env[envgen].attack;
			if (env[envgen].position > 0.99)
			  {
				  env[envgen].phase = PHASE_DECAY;
				  env[envgen].position = 0;
			  }
			break;

		case PHASE_DECAY:
			env[envgen].min = env[envgen].sustain; env[envgen].max = 1; env[envgen].direction = ENV_DOWN;
			curve_point();
			env[envgen].position += env[envgen].decay;
			if (env[envgen].position > 0.99)
			  {
				  env[envgen].phase = PHASE_SUSTAIN;
				  env[envgen].position = 0;
			  }
			break;	

		case PHASE_SUSTAIN:
			  env[envgen].v_out = env[envgen].sustain;
			break;

		case PHASE_RELEASE:
			env[envgen].min = 0; env[envgen].direction = ENV_DOWN;
			curve_point();
			env[envgen].position += env[envgen].release;
			if (env[envgen].position > 0.99)
			  {
				  env[envgen].phase = PHASE_STOPPED;
				  env[envgen].position = 0;
			  }
			break;	
	}
}
		uint16_t dac_out_1 = (uint16_t) (env[0].v_out*4095);
		uint16_t dac_out_2 = (uint16_t) (env[1].v_out*4095);		

		DAC_DHR12R1 = dac_out_1;            // Write output direct to DAC Channel 1 Register (it's faster)
    DAC_DHR12R2 = dac_out_2;            // Write output direct to DAC Channel 2 Register (it's faster)
    //GPIO_BSRR(GPIOA) = (GPIO9 << 16); // And clear the time measurement pin (A9)
}


/*---------------------------------------------------------------------------*/
/** @brief Get the current value of an ADSR generator

@param[in] egen uint8_t  Envelope Generator to get the value from 
@returns float - current value for specified envelope
*/
float adsr_get_val(uint8_t egen)
{
	return env[egen].v_out;
}
