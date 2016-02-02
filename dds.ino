/* Simple Sinewave DDS Routine
 * Inefficient version.  For pedagogical use only!
 * See https://github.com/hexagon5un/barking_dogs_dds_demo 
 * Public Domain
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "fullSine.h"
#include "scale16.h"

#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit)   (_SFR_BYTE(sfr) |= _BV(bit))

#define ACCUMULATOR_STEPS 256
#define SAMPLE_RATE       8000UL
#define NUM_VOICES        1

/* Init functions, defined at the bottom of the file */
static inline void setup_pwm_audio_timer(void);
static inline void setup_sample_timer(void);

const uint16_t sample_max = sizeof(sine_wave);  /* for 8-bit samples */

struct DDS {
	uint16_t increment;
	uint16_t position;
	uint16_t accumulator;
};
volatile struct DDS voices[NUM_VOICES];

ISR(TIMER1_COMPA_vect) {
	PORTB |= (1 << PB1); // debug, toggle pin PB1, Arduino D9
	int16_t total = 0;

	for (uint8_t i = 0; i < NUM_VOICES; i++) {
		total += (int8_t) pgm_read_byte_near(sine_wave + voices[i].position);

		/* Take an increment step */
		voices[i].accumulator += voices[i].increment;
		while (voices[i].accumulator >= ACCUMULATOR_STEPS){
			voices[i].accumulator -= ACCUMULATOR_STEPS;
			voices[i].position++;
			if (voices[i].position == sample_max){    /* loop back around */
				voices[i].position = 0;
			}
		}

	}
	total = total / NUM_VOICES;
	OCR2A = total + 128; // add in offset to make it 0-255 rather than -128 to 127
	PORTB &= ~(1 << PB1); // debug, toggle pin
}

uint16_t scale[] = {C1, D1, E1, F1, G1, A1, B1, C2, D2, E2, F2, G2, A2, B2, C3, D3, E3, F3, G3, A3, B3};

void setup() 
{
	setup_sample_timer();
	setup_pwm_audio_timer();
	set_bit(DDRB, PB1); // debugging 
	
	/* These constants are defined in "scale16.h" */
	voices[0].increment = C2;
}

void loop() 
{  
	static uint8_t i=0;
	voices[0].increment = scale[i];
	i++;
	if (i == (sizeof(scale)/sizeof(scale[0])) ){
		i = 0;
	}
	_delay_ms(500);
}


static inline void setup_sample_timer(){
	// Undo Arduino defaults:
	TCCR1A = 0;
	TCCR1B = 0; 
	TIMSK1 = 0;
	// Set up in count-till-clear mode 
	set_bit(TCCR1B, WGM12); 
	// Sync to CPU clock: fast
	set_bit(TCCR1B, CS10);
	// Enable output-compare interrupt
	set_bit(TIMSK1, OCIE1A);   
	OCR1A = F_CPU / SAMPLE_RATE - 1; 
	sei();
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
	set_bit(DDRB, PB3);  /* or pinMode(11, OUTPUT) in Arduinese */
}
