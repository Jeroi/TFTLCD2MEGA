# TFTLCD2MEGA
ARDUINO UNO TFTLCD library that supports also MEGA boards.

All printing is done now with PORT manipulation rather than digitalWrites. This provides cleanier TTL bits that made MEGA working also without any delays in the library. Old library seemed to work with 6 microsecond delay which made image printing slowdowns.

Added outputPort(char portletter) function if using MEGA chips. There is many PORTS available in MEGA and one can now obtain direct byte to port writes when creating board layout so that port pins are wired directly to lcd controller 8bit inputs.

Library confirmed to work with 2.4" LCD display with SD card shield:

https://www.dropbox.com/s/p40ol6cvzoo3eiw/arduino-mega.png?dl=0

