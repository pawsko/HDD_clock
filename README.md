# My first embedded project #
## Introduction ##
Some time ago I saw an amazing clock based on HDD on Youtube. There was dedicated clock face made from original plate with drilled numbers
from 0 to 9 and colon sign. Under plate there were LEDs. Clock face was rotating very fast, exactly 5400RPM like regular HDD. LEDs were blinked
exectly right in time when right number was above right LED. I was thinking this project is perfect to replicate for my self and practise C programming:)
Clock which I saw had only hours and minutes. I wanted to go a step further and add seconds as well.

It took me some of time to figure out how to approach to this topic.
The easiest part for me was design hardware. I took broken HDD, removed all platters, measured their diameters and made a decision that there will be 
12 signs on PCB.
Why 12? because of dividing. 

360 degrees / 11 signs = 32.72727272.... degrees per sign

360 / 12 = 30 degrees per sign. Very nice.

I wasn't shure about shape of controller, so I split electronics to two part. Controller and LED driver. I planned to use evaluation board 
with FPGA or microcontroller on board and design only board with LED, power supply circuits and buttons. 
FPGA is obvoius for this solution but I have no knowledge and experience in this area.
## First version (prototype) ##
In first attempt I used NUCLEO-F103RB. Cheap and very popular evaluation board with STM32F103 microcontroller. I utylized there all counters. 
Each digit has own. One additional counter measures one complete revolution.
There is an interrupt for detect zero position of clock face.
Brushless motor controller was taken from HDD so it spinup motor only for 10 second and expected signal from magnetic heads (were removed). Additionally once per second reduced speed for a while.
Despite this it worked. After 10 seconds motor has stopped. Probably disk controller reported error to host (BIOS in PC).
There was some issue with synchronisation while colons were blinked.
## Second version ##
I integrated two parts of electronics to one board and I added Real Time Clock programmed through UART with backup battery, brushless motor controller,
button to change mode clock/callendar (last video). I changed soldermask from regular green to black (looks like dark brown).
## Opened issues ##
1. Unfortunatelly motor is noisy.
2. Contrast is insufficient during sunlight. LEDs are super bright but clock face builded from FR4 provides low light transmision. Second wersion with black solder mask is much more effective, but it's still not enough.
3. Clock face is unbalanced because of heteregenous material and digits. Reminder: 5400RPM.
4. Best effect is in the dark room (at night) but take a look at point one :(

## Photos ##
<img src="https://github.com/pawsko/HDD_clock/blob/master/Media/v1_proto.jpg" width="200"><img src="https://github.com/pawsko/HDD_clock/blob/master/Media/v1.9.jpg" width="200"><img src="https://github.com/pawsko/HDD_clock/blob/master/Media/v2%20i%20v3.jpg" width="400">

<img src="https://github.com/pawsko/HDD_clock/blob/master/Media/Clock_face_top.jpg" width="200"><img src="https://github.com/pawsko/HDD_clock/blob/master/Media/Clock_face_bottom.jpg" width="200"><img src="https://github.com/pawsko/HDD_clock/blob/master/Media/Clock_face_bottom_proto.jpg" width="200">

## Links to short video ##
https://youtube.com/shorts/ItIJTu948P8

https://youtube.com/shorts/eFmTAhwEkuo

V2 with old clock face. 

https://www.youtube.com/watch?v=KfqW7rv5enw

## Titbit ##
How much do mistakes cost??

<img src="https://github.com/pawsko/HDD_clock/blob/master/Media/mistake.jpg" width="300">

