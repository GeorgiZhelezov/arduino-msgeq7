#include <Adafruit_NeoPixel.h>

unsigned short leftMSGEQ[7]; //arrays for storing MSGEQ7 data
unsigned short rightMSGEQ[7];
unsigned short strobePins[2] = { 2,4 }; //IO pins
unsigned short resetPins[2] = { 3,5 };
unsigned short analogPins[2] = { 0,1 };
unsigned short outputPins[2] = { 8,9 }; //data to the LED strips
unsigned short numLEDs = 119; //per block
unsigned short LEDsPerStrip = numLEDs / 7;
unsigned short minMAPlimit = 0; //limits used to convert MSGEQ7 values
unsigned short maxMAPlimit = LEDsPerStrip;
unsigned short noiseFilter = 90; //used to get rid of unwanted noise
unsigned short brightness = 20; //LED strips brightness
unsigned short readDataDelay = 3; //delay after reading MSGEQ7 data
unsigned short readConsoleDelay = 2; //delay while reading from the console
unsigned short baudRate = 9600;
unsigned short serialInputNumber; //read input command is stored here
unsigned short lastCommand; //keep track of the last command
unsigned short command = 2; //set a default value of 2

//strip objects
Adafruit_NeoPixel strip0 = Adafruit_NeoPixel(numLEDs, outputPins[0], NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(numLEDs, outputPins[1], NEO_RGB + NEO_KHZ800);

//some color samples
uint32_t white = strip0.Color(255, 255, 255);
uint32_t black = strip0.Color(0, 0, 0);

uint32_t red = strip0.Color(255, 0, 0);
uint32_t green = strip0.Color(0, 255, 0);
uint32_t blue = strip0.Color(0, 0, 255);

uint32_t orange = strip0.Color(255, 50, 0);

void setup()
{
	Serial.begin(baudRate);
	Serial.setTimeout(readConsoleDelay); //set the delay when reading from the console

	pinMode(strobePins[0], OUTPUT); //set pin modes
	pinMode(strobePins[1], OUTPUT);

	pinMode(resetPins[0], OUTPUT);
	pinMode(resetPins[1], OUTPUT);

	pinMode(analogPins[0], INPUT);
	pinMode(analogPins[1], INPUT);

	digitalWrite(resetPins[0], HIGH); //reset the MSGEQ7s
	digitalWrite(resetPins[1], HIGH);

	digitalWrite(resetPins[0], LOW);
	digitalWrite(resetPins[1], LOW);

	strip0.begin(); //initialize the strips
	strip1.begin();

	strip0.setBrightness(brightness); //set their brightness
	strip1.setBrightness(brightness);
}

void loop()
{
	readData();
	setLEDs();

	readConsole();

	strip0.show(); //update the strips
	strip1.show();
}

void readData() //reads MSGEQ7 data
{
	for (short i = 0; i < 7; i++)
	{
		digitalWrite(strobePins[0], LOW); //enable both MSGEQ7s
		digitalWrite(strobePins[1], LOW);

		leftMSGEQ[i] = analogRead(analogPins[0]); //read data
		rightMSGEQ[i] = analogRead(analogPins[1]);

		digitalWrite(strobePins[0], HIGH); //disable both and on the next enable it will spit out values for 
		digitalWrite(strobePins[1], HIGH); //the other frequency bands

		leftMSGEQ[i] = constrain(leftMSGEQ[i], noiseFilter, 1023); //getting rid of that noise
		rightMSGEQ[i] = constrain(rightMSGEQ[i], noiseFilter, 1023);

		leftMSGEQ[i] = map(leftMSGEQ[i], noiseFilter, 1023, minMAPlimit, maxMAPlimit); //convert to more suitable values
		rightMSGEQ[i] = map(rightMSGEQ[i], noiseFilter, 1023, minMAPlimit, maxMAPlimit);

		delay(readDataDelay);
		//DEBUGGING
		/*  Serial.print(leftMSGEQ[i]);
		  Serial.print(" ");*/
	}
	/*Serial.print("\t\t");
	for (short i = 0; i < 7; i++)
	{
	  Serial.print(rightMSGEQ[i]);
	  Serial.print(" ");
	}
	Serial.println();*/
	//END DEBUGGING
}

void setLEDs() //set which LEDs should be on/off
{
	for (short i = 0; i < 7; i++) //i is like an LED colomn
	{
		for (short j = 0; j < LEDsPerStrip; j++) //j is like an LED row
		{
			strip0.setPixelColor(calcMethod(i, j), condition(i, j, 0) ? setLEDcolor(i, j, 0) : setLEDoff(i, j, 0));
			strip1.setPixelColor(calcMethod(i, j), condition(i, j, 1) ? setLEDcolor(i, j, 1) : setLEDoff(i, j, 1));
		}
	}
}
//stripNum is used to differentiate between the first strip (strip0) and the second (strip1)
//because both strips are updated in the same function

void readConsole() //read command from the console
{
	serialInputNumber = Serial.parseInt(); //read a number(command) from the console
	if (serialInputNumber != 0) //Serial.parseInt() returns 0 if nothing else is read
	{
		lastCommand = command;
		command = serialInputNumber;
	}
}

short calcMethod(short i, short j) //calculate the exact position of the LED i and j are looking at
{
	switch (i % 2)
	{
	case 0:
		return LEDsPerStrip * i + j; //even strips are looking up
		break;
	case 1:
		return LEDsPerStrip * (i + 1) - j; //odd strips are looking down so you calculate in reverse
		break;
	}
}

short condition(short i, short j, short stripNum) //used to differentiate between the setLEDcolor and setLEDoff
{
	return stripNum == 0 ? j < leftMSGEQ[i] : j < rightMSGEQ[i];
}


uint32_t setLEDcolor(short i, short j, short stripNum) //returns the desired color for the .setPixelcolor function
{
	switch (command)
	{
	case 1: //reserved for menu
		Serial.print("Command list:\n\n1.Print command list\n2.Orange\n");
		command = lastCommand;
		break;
	case 2: //default value of command
		return orange;
		break;
	default: //all LEDs will be black if no condition in the switch-case is met
		return black;
		break;
	}
}

uint32_t setLEDoff(short i, short j, short stripNum) //used to set the rest of the LEDs to black(off)
{
	return black;
}
