# barking_dogs_dds_demo

A quick silly demo of one way to implement DDS sample playback on an AVR/Arduino.  It plays up to four simultaneous dog-barks at different pitches. But your job is to make something cooler out of it, either by replacing the "bark" sample, or by adding a keyboard, or...

The samples themselves are 8-bit @ 8kHz mono.  If you want to replace them, just save your sample as a mono, 8kHz, 8-bit WAV file (e.g. with Audacity or sox) and then you can tweak the included Python file to make a suitable C header with your data.

## Download for Arduino, or compile in C

To run this on Arduino, download the [linked zip file](https://github.com/hexagon5un/barking_dogs_dds_demo/archive/master.zip) and move it to your sketches directory.  Rename it to "dogs" and fire up Arduino.  You should be good to go.

If you're interested in the synthetic waveforms code, I've pushed up a branch for that too.  Download [this zip file](https://github.com/hexagon5un/barking_dogs_dds_demo/archive/all_waveforms_demo.zip) and move it to "dds" in your sketches directory.  (It's got some sweet Python band-limited waveform synthesis code if I do say so myself.)

I wrote the code in pretty agnostic C so that it should compile just fine once you replace `setup()` and `loop()` with `main()`.  That's what you'll find in the `dogs_in_c` directory.  Works for me.  Also included, at no extra charge, my Arduino-alike Makefile, and a generic AVR-GCC Makefile (with _far_ too many frills) in their respective directories.  Feel free to ignore or copy.

## PWM

The PWM rate is as fast as it could be: 16 MHz / 256 = 62 kHz.  You're still going to want to filter out some of the high-frequency hash.  I used a single-pole RC filter: a 1K resistor from the AVR's output with a 0.1 uF capacitor to ground.  Not high-tech, but gets the job done.

Audio out is on pin 11, PB3 / OC2A.  It's directly toggled by Timer2 in hardware, and is "jitter-free".  Or at least it looks pretty good on my scope. 

## Interrupts

The action in the code is all in the periodic interrupt routine triggered on Timer 1.  It reads each sample from memory, adds them together, and divides by the number of voices.  If a "bark"'s increment value is set to zero, it doesn't play.  If it's non-zero, it takes off.  That's all there is to it, really.  This means that you can trigger a bark by setting the pitch: `bark[1].increment = 256` will set one off at the sampled pitch.

The 4-voice lookup and mixing takes about 10-20% of the processing time, which is a heck of a lot to be running inside a blocking interrupt.  But if you consider that sample-loading is probably the highest priority task, and that basically you're going to want to scan keys or something equally trivial with the rest of the CPU time, it's not a problem.  

If you want to reassure yourself of this, I left my pin-toggling debug code in for you -- Arduino pin 9 (PB1) is pulled high during the ISR.  Running this pin into a scope and watching the duty cycle is a good way to double-check that all's well.

## Optimizations

There are _tons_ of optimizations that could be applied to this code.  It goes with an introductory [article on DDS I wrote for Hackaday](http://hackaday.com/2016/02/12/embed-with-elliot-audio-playback-with-direct-digital-synthesis/), and it's built for comfort.  It ain't built for speed. 


## Further work?

If you took a sample, split it up into attack, sustain, and decay phases, and then added a couple if() statements in the ISR, you'd have a decent general purpose (8-bit, 8kHz) sample player. 


