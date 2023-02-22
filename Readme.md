https://youtube.com/shorts/isAoaTnkfzQ


# An overengineered Catan Dice pretending to be speaking slot machine

Just to learn Raspberry Pi Pico, I've created this random generator I use now to play The Settlers of Catan.

## Main features

- Generating two random numbers from 1 to 6. The randomness is based on some none deterministic factors such as:

    - shifting an internal register
    - random time between button pressed
    - value of a two floating ADC inputs

- Animate generated numbers. Here pico pretends to be a slot machine with some simple sound efects

- Speak out result (showing is boring and pic has a lot of memory for some wav-data)

- Show statistics from this game and all prevoius saved on a sd-card, to check if it is really [random](https://dilbert.com/strip/2001-10-25). 


## Miscelenius  

- Check 3D folder for enclousure (you may need a file to let usb and sd card in)

- Check docs & wiring folder for schematics and some pictures from DIY

- BOM:

    - RPI Pico
    - LCD 1.8" with SD card slot
    - Phone mini loudspeaker 
    - Rotary Encoder
    - few resistors, capcitors



... Some further details in preperation or on a request.

