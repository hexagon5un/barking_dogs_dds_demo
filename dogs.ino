/* Random Dogs Barking
 * a direct-digital synthesis "tutorial"
 * for the AVR, but will compile on an Arduino 
 * Released public domain, because it's absurd, frankly.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "WAV_bark.h"
#include "scale16.h"

#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define set_bit(sfr, bit)   (_SFR_BYTE(sfr) |= _BV(bit))

#define ACCUMULATOR_INCREMENT 2048
#define SAMPLE_RATE           8000UL
#define NUM_BARKERS	          4

/* Init functions, defined at the bottom of the file */
static inline void setup_pwm_audio_timer(void);
static inline void setup_sample_timer(void);

const uint16_t bark_max = sizeof(WAV_bark)-1;

struct Bark {
	uint16_t increment = ACCUMULATOR_INCREMENT;
	uint16_t position = 0;
	uint16_t accumulator = 0;
};
volatile struct Bark bark[NUM_BARKERS];

ISR(TIMER1_COMPA_vect) {
	PORTB |= (1 << PB1); // debug, toggle pin
	uint16_t total = 0;

	for (uint8_t i = 0; i < NUM_BARKERS; i++) {
		total += pgm_read_byte_near(WAV_bark + bark[i].position);

		if (bark[i].position < bark_max){    /* playing */
			bark[i].accumulator += bark[i].increment;
			while (bark[i].accumulator >= ACCUMULATOR_INCREMENT){
				bark[i].position++;
				bark[i].accumulator -= ACCUMULATOR_INCREMENT;
			}
		} else {  /*  done playing, reset and wait  */
			bark[i].position = 0;
			bark[i].increment = 0;
		}
	}
	total = total / NUM_BARKERS;
	OCR2A = total; 
	PORTB &= ~(1 << PB1); // debug, toggle pin
}

// These constants are defined in "scale16.h"
uint16_t scale[] = {C1, E1, G1, C2, E2, G2, C3};
const uint8_t scale_max = sizeof(scale)/sizeof(scale[0]);

void setup() 
{
	setup_sample_timer();
	setup_pwm_audio_timer();

	set_bit(DDRB, PB1); // debugging 

	/* Bark once at native pitch */
	bark[0].increment = ACCUMULATOR_INCREMENT;
	_delay_ms(1000);

	/* Demo the entire scale */
	for (uint8_t i=0; i < scale_max; i++){
		bark[i % NUM_BARKERS].increment = scale[i];
		_delay_ms(150);
	}
}

void loop() 
{  
	_delay_ms(120);
	for (uint8_t i=0; i < NUM_BARKERS; i++){
		if (bark[i].increment == 0) { /* only start another if done playing */
			if (rand() > 127){        /* chance of barking: rand is 0-255   */
				bark[i].increment = scale[ rand() % scale_max ];
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

