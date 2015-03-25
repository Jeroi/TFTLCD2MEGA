# TFTLCD2MEGA
ARDUINO UNO TFTLCD library that supports also MEGA boards.

I added #define SPEED_DELAY 6 to the TFTLCD.cpp that adds READ/WRITE operation delays. I found out that 6 microsecond delay were minimum for our LCD to work. In case your TFT does not work, try to increase this in steps.

LIbrary confirmed to work with 2.4" LCD display with SD card shield:

https://dl-web.dropbox.com/get/arduino-mega.png?_subject_uid=144804207&w=AADaEzj09TqeiFvG1VibIAC5BZ3unFjSiHgCT18SD8Vs9A">

