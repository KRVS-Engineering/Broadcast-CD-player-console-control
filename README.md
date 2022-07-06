# Broadcast-CD-player-console-control
Convert GPIO contact closure to TCP to start/stop broadcast CD players from console

Professional CD players have remote ports on the back that can trigger start/stop commands.  These ports are tied into the on-air audio console so that when you turn on the "channel", the CD players start automatically.  Just reduces the number of things you have to physically do when hosting a show.  New CD players do not have this feature and have switched to RS-232 or TCP for their remote commands.  This project hopes to bridge the dry contact GPIO from our existing console to a TCP command via an ESP-32.

Hardware:
Denon DN-500BD MKII Blueray/CD Player (Broadcast grade) https://denon-professional.de/sites/default/files/2018-06/DN-500BD_MKII_Protocol_Guide_1.0.pdf
OLIMEX ESP32-POE  https://www.olimex.com/Products/IoT/ESP32/ESP32-POE/resources/ESP32-POE-GPIO.png
Wheatstone E-6 console with Sat Cages (Dry contact logic (GPIO)) Start command = closed Stop command = open


