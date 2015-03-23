# TFTLCD2MEGA
ARDUINO UNO TFTLCD library that supports also MEGA boards.

Me and my school mate were creating DSP controller with inexpensive 2.4" TouchScreen with SD card from ebay. Originally misc.ws Mike's work was essential to get us going on UNO. But we soon found out that the LCD uses UNO's SDA and SDC A3 nad A4 pins so we needed more pins. Thus we bought Arduino MEGA rev.3. It were found that the LCD did not work out of the box similary like in Arduino. Mike stated on another post that hes addition's to the library made it work with Mega also but interestely this library did not work for us either.

Since it was obivous that we needed SDA and SDC and if MEGA is not working, we are not getting anywhere until it works and then I tought, I will need to battle with every library before I can go sleep and after all of them, UTFT, STFT I found one UTFT clone that were made to work with this board, but is was too weak, all white hazy stripes all around. Then after 6 hours, I looked once more what it goin on with the TFTLCD library. 

I commented out all non mega ifdefs from the code what so ever. It did not work. Finally I increased write function 5 microsecond delay to 6 and BANG!!! My TFT came alive!

So this github page is devoted to TFTLCD2MEGA and if there is decent coders available who are willing to improve this librarys performance with MEGA boards the help would be very appreciated.

Essentially I added #define SPEED_DELAY 6 to the TFTLCD.cpp that adds READ/WRITE operation delays. I found out that 6 microsecond delay were minimum for our LCD to work. In case your TFT does not work, try to increase this in steps.

