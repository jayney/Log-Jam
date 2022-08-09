#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include "responsive_adc.h"
#include "defines.h"









static void clock_setup(void)
{
	// Setup main clock for 180MHz (max frequency) from 8MHz external source
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_180MHZ]);

	//Enable clock for GPIO A/B/C
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);

	// Enable clocks for USART2 ,DAC and ADC1 
	rcc_periph_clock_enable(RCC_USART2);
	rcc_periph_clock_enable(RCC_DAC);
	rcc_periph_clock_enable(RCC_DMA2);
	rcc_periph_clock_enable(RCC_ADC1);
}



static void gpio_setup(void)
{
	// Digital Output for timing measurements
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9);
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
	gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO13);
	gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO4);	
	gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO5);

	// ADC Channels

	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);	// Channel 0
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);	// Channel 1

	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO6);	// Channel 6
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO7);	// Channel 7
	gpio_mode_setup(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);	// Channel 8
	gpio_mode_setup(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);	// Channel 9

	gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);	// Channel 10
	gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);	// Channel 11

	gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO2);	// Channel 12
	gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO3);	// Channel 13
	gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO4);	// Channel 14 
	gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO5);	// Channel 15

}


static void adc_setup(void)
{
	adc_power_off(ADC1);
    adc_set_clk_prescale(ADC_CCR_ADCPRE_BY8);
    adc_set_right_aligned(ADC1);
    adc_set_resolution(ADC1, ADC_CR1_RES_12BIT);
    adc_disable_external_trigger_regular(ADC1);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_480CYC);

	adc_enable_scan_mode(ADC1);
//	adc_set_continuous_conversion_mode(ADC1);

	uint8_t channel_sequence[] = { 0,1,6,7,8,9,10,11,12,13,14,15 };  
	adc_set_regular_sequence(ADC1, 12, channel_sequence);

//	adc_enable_dma(ADC1);

// There doesn't seem to be an API function, but the DDS bit needs to be 
// set for continuous DMA operation, discovered by trial and error!
//	ADC1_CR2 |= ADC_CR2_DDS;	
								
	adc_power_on(ADC1);
//	uint32_t i;
//	for (i=0;i<100000;i++) __asm__("NOP");
//	adc_start_conversion_regular(ADC1);
}





static void dac_setup(void)
{
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO5);
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO4);
	dac_disable(CHANNEL_1);	
	dac_disable(CHANNEL_2);
	dac_disable_waveform_generation(CHANNEL_1);
	dac_disable_waveform_generation(CHANNEL_2);
	dac_enable(CHANNEL_1);
	dac_enable(CHANNEL_2);
	dac_set_trigger_source(DAC_CR_TSEL2_T7);
}




static void timer_setup(void)
{
/* 
* All of these are pretty self explanatory.
* Basically sets up a basic timer that will generate an interrupts
* at the sample rate.
*
*/
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





static void usart_setup(void)
{
//	Setup GPIO pins for USART2 transmit.
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);

//	Setup USART2 TX pin as alternate function.
	gpio_set_af(GPIOA, GPIO_AF7, GPIO2);

	usart_set_baudrate(USART_CONSOLE, 115200);
	usart_set_databits(USART_CONSOLE, 8);
	usart_set_stopbits(USART_CONSOLE, USART_STOPBITS_1);
	usart_set_mode(USART_CONSOLE, USART_MODE_TX);
	usart_set_parity(USART_CONSOLE, USART_PARITY_NONE);
	usart_set_flow_control(USART_CONSOLE, USART_FLOWCONTROL_NONE);

//	Finally enable the USART
	usart_enable(USART_CONSOLE);
	//printf("Started!\n");
}










void init_hw(void)
{
	clock_setup();
	gpio_setup();
	adc_setup();
	dac_setup();
	timer_setup();
	usart_setup();
}