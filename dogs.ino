// dsp-D8 Drum Chip (c) DSP Synthesizers 2015
// Free for non commercial use

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "WAV_bark.h"

#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define SAMPLE_RATE 8000UL

static inline void setup_pwm_audio_timer(void);
static inline void setup_sample_timer(void);

const uint16_t bark_max = sizeof(WAV_bark)-1;
volatile uint16_t bark_increment[4]={255,255,255,255};

ISR(TIMER1_COMPA_vect) {
	PORTB |= (1 << PB1); // debug, toggle pin
	static uint16_t bark_position[4] = {0,0,0,0};
	static uint16_t bark_accumulator[4] = {0,0,0,0};
	uint16_t total = 0;

	for (uint8_t i = 0; i < 4; i++) {
		total += pgm_read_byte_near(WAV_bark + bark_position[i]);

		if (bark_position[i] < bark_max){    /* playing */
			bark_accumulator[i] += bark_increment[i];
			while (bark_accumulator[i] >= 255){
				bark_position[i]++;
				bark_accumulator[i] -= 255;
			}
		} else {  /*  done playing, reset and wait  */
			bark_position[i] = 0;
			bark_increment[i] = 0;
		}
	}
	total = total / 4;
	OCR2A = total; 
	PORTB &= ~(1 << PB1); // debug, toggle pin
}

void setup() 
{
	setup_sample_timer();
	setup_pwm_audio_timer();

	set_bit(DDRB, PB1); // debugging 
	Serial.begin(115200);
}


void loop() 
{  
	_delay_ms(200);
	for (uint8_t i=0; i < 4; i++){
		if (bark_increment[i] == 0) {   /*  if done playing */
			if (random(0,5) > 3){
				bark_increment[i] = random(128, 512); 
			}
		}
	}
}


static inline void setup_sample_timer(){
	// Undo Arduino defaults:
	TCCR1A = 0;
	TCCR1B = 0; 
	TIMSK1 = 0;
	// Set up in count-till-clear mode 
	set_bit(TCCR1B, WGM12); 
	// Set to internal clock
	set_bit(TCCR1B, CS10);
	// Enable output-compare interrupt
	set_bit(TIMSK1, OCIE1A);   
	sei();
	OCR1A = F_CPU / SAMPLE_RATE; 
}

static inline void setup_pwm_audio_timer(){
	// Undo Arduino defaults:
	TCCR2A = 0;
	TCCR2B = 0; 
	// Set fast PWM mode
	TCCR2A |= _BV(WGM21) | _BV(WGM20); 
	// With output on OC2A / Arduino pin 11
	set_bit(TCCR2A, COM2A1);
	// Set fastest clock speed: ~62kHz @ 16MHz
	set_bit(TCCR2B, CS20);
	// Signal is symmetric around 127
	OCR2A = 127;
	// output on pin 11 -- OC2A 
	pinMode(11, OUTPUT); // PB3
}

