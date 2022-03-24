  Comple£ºmake
  Run: ./bme280
  
  This Demo is tested on Raspberry PI 3B+
  you can use I2C or SPI interface to test this Demo
  When you use I2C interface,the default Address in this demo is 0X77
  When you use SPI interface,PIN 27 define SPI_CS
  
1) Set Date/Time to match (to the minute) on both computers

2) Set Date/Time on the Raspberry Pi to match
	a) Open the Command Terminal (top left of the screen)
	b) Enter the Date/Time like below and press enter: 

		sudo date -s "Thurs Sept 12 13:15:00 HST"
	
	c) Close the command terminal

3) Put the sensor where you want it

4) Right-Click on the folder "Data_Logger" on the Desktop
	a) Select "Open in Terminal"
	b) Type the following and press enter:

		gcc main.c bme280.c -o bme280

	c) Type the following and press enter:
	
		./bme280
	
	d) It should say 

		"Data Logger started successfully and is currently running..."

	e) Remove the mouse, keyboard, monitor and leave running for up to 9 days 
