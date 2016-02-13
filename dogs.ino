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
#define LED_ON              set_bit(PORTB, PB1) // debug, pin PB1, Arduino D9
#define LED_OFF             clear_bit(PORTB, PB1)
#define LED_INIT	        set_bit(DDRB, PB1) /* debugging -- this pin toggled in ISR  */

  /* Note: If ACCUMULATOR_STEPS is a power of two, 
   * the math works fast and all is well.
   * If not, the division and modulo by ACCUMULATOR_STEPS kills it.
   * */
#define ACCUMULATOR_STEPS 2048 
#define SAMPLE_RATE           8000UL
#define NUM_BARKERS	          4

/* Init functions, defined at the bottom of the file */
static inline void setup_pwm_audio_timer(void);
static inline void setup_sample_timer(void);
static inline void init_barking_leds(void);

const uint16_t bark_max = sizeof(WAV_bark);

struct Bark {
	uint16_t increment = ACCUMULATOR_STEPS;
	uint16_t position = 0;
	uint16_t accumulator = 0;
};
volatile struct Bark bark[NUM_BARKERS];

ISR(TIMER1_COMPA_vect) {
	LED_ON; // debug, toggle pin
	int16_t total = 0;

	for (uint8_t i = 0; i < NUM_BARKERS; i++) {
		total += (int8_t)pgm_read_byte_near(WAV_bark + bark[i].position);

		if (bark[i].position < bark_max){    /* playing */
			bark[i].accumulator += bark[i].increment;
			bark[i].position += bark[i].accumulator / ACCUMULATOR_STEPS; 
			bark[i].accumulator = bark[i].accumulator % ACCUMULATOR_STEPS;
		} else {  /*  done playing, reset and wait  */
			bark[i].position = 0;
			bark[i].increment = 0;
			clear_bit(PORTC, i);  /* Turn off goofy LED */
		}
	}
	total = total / NUM_BARKERS;
	OCR2A = total + 128; // add in offset to make it 0-255 rather than -128 to 127
	LED_OFF; // debug, toggle pin
}

// These constants are defined in "scale16.h"
uint16_t scale[] = {NOTE_C1, NOTE_E1, NOTE_G1, NOTE_C2, NOTE_E2, NOTE_G2, NOTE_C3};
const uint8_t scale_max = sizeof(scale)/sizeof(scale[0]);

void setup() 
{
	setup_sample_timer();
	setup_pwm_audio_timer();
	init_barking_leds();

	set_bit(DDRB, PB1); // debugging 

	/* Bark once at native pitch */
	bark[0].increment = ACCUMULATOR_STEPS;
	bark[1].increment = ACCUMULATOR_STEPS;
	_delay_ms(300);
	bark[0].increment = ACCUMULATOR_STEPS;
	bark[1].increment = ACCUMULATOR_STEPS;
	_delay_ms(500);
	bark[0].increment = ACCUMULATOR_STEPS;
	_delay_ms(1000);

	/* Demo the entire scale */
	for (uint8_t i=0; i < scale_max; i++){
		bark[i % NUM_BARKERS].increment = scale[i];
		_delay_ms(400);
	}
}

void loop() 
{  
	for (uint8_t i=0; i < NUM_BARKERS; i++){
		if (bark[i].increment == 0) { /* only start another if done playing */
			if ((uint8_t) rand() > 185){        
				bark[i].increment = scale[ rand() % scale_max ];
				set_bit(PORTC, i);   /* Turn on LED for light show. */
			}
		}
	}
	_delay_ms(200);
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

static inline void init_barking_leds(void){
	set_bit(DDRC, PC0);
	set_bit(DDRC, PC1);
	set_bit(DDRC, PC2);
	set_bit(DDRC, PC3);
}


