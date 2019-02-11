# SATB #
### _a four-voice pentatonic synthesizer_ ###
_using teensy 3.2 and audio board_     

## recycle! ##
you can build this into any box:

![SATB](https://github.com/dallinw/SATB/blob/master/SATB.jpg)


## parameters: ##
* frequency; conformed to C major pentatonic scale over 5 octaves
* volume
* voice enable/disable
* waveform 
  * sine
  * triangle
  * variable triangle
  * sawtooth
  * square

## hardware: ##
two 10k pots, four buttons and LEDs, one 5-way Fender switch, one 1/4" mono output (or just use the built-in headphone output on the audio board).

### pins: ###

the pin choices may look esoteric; this is because certain pins are unavailable because they're already in use by the audio board for i2s & i2c.


i/o           | pins            | resistors
------------- | ----------------| -----------------
buttons       | 3, 4, 5, 8      | 10k pull-down
pots          | A0, A4          |
5-way switch  | 0, 1, 2         | 10k pull-down (3)
LEDs          | 21, 20, 17, 15  | 


recommended: double-decker headers from [here](https://www.pjrc.com/store/header_14x1_d.html "PJRC store"). 






