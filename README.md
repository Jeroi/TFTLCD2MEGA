# TFTLCD2MEGA
ARDUINO UNO TFTLCD library that supports also MEGA boards.

This is an internationalised version of TFTLCD. This means no char string can be provided directly to drawString("string here") function. 
One must use:
 - unsigned char localizedString[] = "My text displays extented ascii with escapes \x98\x89";

Meaning:

 - "My text displays extented ascii with escapes öä"

Extended letter values are in glcdfont.c and one must convert it to HEX to input escapes to the text string.

Improvements to the TFTLCD lib:
 - All printing is done now with PORT manipulation rather than digitalWrites. This provides cleanier TTL bits that made MEGA working also without any delays in the library. Old library seemed to work with 6 microsecond delay which made image printing slowdowns.
 - Added outputPort(char portletter) function if using MEGA chips. There are many PORTS available in MEGA and one can now obtain fastest wrtiting with direct byte to port writes when creating board layout so that port pins are wired directly to lcd controller 8bit inputs.

Library confirmed to work with 2.4" LCD display with SD card shield:

![Library works with Mega](https://raw.githubusercontent.com/Jeroi/TFTLCD2MEGA/screenshot/arduino-mega.png)
