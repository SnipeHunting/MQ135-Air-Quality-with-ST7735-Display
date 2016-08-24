
/****************************************************************
/*
Based on source code from G.Krocher https://github.com/GeorgK/MQ135  and his great detalied work provided at:
https://hackaday.io/project/3475-sniffing-trinket/log/12363-mq135-arduino-library

Modified code to include an intrerface with ST7355 display and DHT22 temperature/ humidity sensor

-  The controller circuit is based on an ATM328p basic configuration (see for example here:
   http://www.electroschematics.com/8064/diy-arduino-making-your-own-arduino/).  
   You can use the internal crystal, but make sure that the fuses are programmed accordingly.

-  The display is 1.8" ST7735 display, that can be connected as described in this drawing:  
   http://www.vwlowen.co.uk/arduino/AD9850-waveform-generator/AD9850-waveform-generator.htm.  

   Adafruit library was used for the display:  https://github.com/adafruit/Adafruit-ST7735-Library

-  The MQ-135 sensor can be connected to any available analog input


All licence terms outlined by the original license are reserved (https://github.com/GeorgK/MQ135/blob/master/LICENSE)

*/
/************************************************************************** /

/**************************************************************************/
/*!
@file     MQ135.cpp
@author   G.Krocker (Mad Frog Labs)
@license  GNU GPLv3
First version of an Arduino Library for the MQ135 gas sensor
TODO: Review the correction factor calculation. This currently relies on
the datasheet but the information there seems to be wrong.
@section  HISTORY
v1.0 - First release
*/
/**************************************************************************/

#include <MQ135.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <math.h>
#include <DHT.h>

// initialize display
#define TFT_DC            9     // RS/DC
#define TFT_RST           8     // RES
#define TFT_CS           10     // CS
static Adafruit_ST7735 TFT = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define DHTTYPE DHT22   // DHT 22  (AM2302)

const unsigned int BAR_Blue = TFT.Color565(0, 192, 204);
const unsigned int BAR_Green  = TFT.Color565(128,255,0);
const unsigned int BAR_Yellow  = TFT.Color565(255,255,102);
const unsigned int BAR_Orange  = TFT.Color565(255,128,0);
const unsigned int BAR_Red  = TFT.Color565(204,0,0);

uint8_t sensorAnalogPin = A0;
// int sensorDigitalPin = 2;
int ledPin = 13;
int sensorAnalogValue = 0;
int sensorSum = 0;
int DHT22_Sensor_pin = 2;
float displayPPM = 0;
float lastReadPPM = 0;

float humidity = 0;
float lastHumidity = 0;
float temperatureF = 0;
float temperatureC = 0;
float lastTemperatureF = 0;
float lastTemperatureC = 0;

float rzero = 0;

int tempTrend = 0;
int humidityTrend = 0;
int ppmTrend = 0;

bool dhtError = false; // sensor reading error indication

DHT dht(DHT22_Sensor_pin, DHTTYPE);

double barLength = 0;

int counter = 0; // indicates when to display summary
bool alarmDetected = false;

unsigned long time; // for timer


void setup() {

  
	pinMode(ledPin, OUTPUT);
	pinMode(sensorAnalogPin, INPUT);
	pinMode(DHT22_Sensor_pin, INPUT);

	digitalWrite(ledPin, LOW);    // turn the LED OFF

   // initialize a ST7735S 
	TFT.initR(INITR_BLACKTAB);
	TFT.fillScreen(ST7735_BLACK);
	TFT.setTextColor(ST7735_WHITE);
	TFT.setRotation(TFT.getRotation() + 1);  // side rotation

	dht.begin();
	readTempHumid();

	time = millis();

}

void loop()

{
	MQ135 gasSensor = MQ135(sensorAnalogPin);

	float correctionFactor = gasSensor.getCorrectionFactor(temperatureC, humidity);

	for (int i = 0; i <= 1000; i++)
	{
		float ppm = gasSensor.getPPM();

		if (ppm > displayPPM)
			displayPPM = ppm;
	}


		if (displayPPM <= 40000)
			barLength = 63.517413344850005797599828173901*log10(displayPPM / 250);
		else
			barLength = 140;

		readTempHumid();
		if (abs(lastHumidity - humidity)>0.3 || abs(lastTemperatureF - temperatureF)>0.2 || abs(lastReadPPM - displayPPM)>1)
		{

			//update humidity trend
			if (humidity > lastHumidity)
				humidityTrend = 1;
			else
				if (humidity < lastHumidity)
					humidityTrend = 2;
				else
					humidityTrend = 0;

			//update temperature trend
			if (temperatureF > lastTemperatureF)
				tempTrend = 1;
			else
				if (temperatureF < lastTemperatureF)
					tempTrend = 2;
				else
					tempTrend = 0;

			//update ppm trend
			if (displayPPM > lastReadPPM)
				ppmTrend = 1;
			else
				if (displayPPM < lastReadPPM)
					ppmTrend = 2;
				else
					ppmTrend = 0;


			updateDisplay();
			lastHumidity = humidity;
			lastTemperatureF = temperatureF;
			lastReadPPM = displayPPM;

			time = millis(); // reset the timer
		}
		
// update timer after ~60 sec if no other changed

		if (millis() - time >60000)
		{
			updateDisplay();
			time = millis();
		}

		rzero = gasSensor.getRZero();

		displayPPM = 0;
}


static void readTempHumid()
{
	humidity = dht.readHumidity();
	// Read temperature as Celsius
	temperatureF = dht.readTemperature(true);
	temperatureC = dht.readTemperature();

	// Check if any reads failed and exit early (to try again).
	if (isnan(humidity) || isnan(temperatureF)) {
		dhtError = true;
		return;
	}
	
}

static void updateDisplay()
{
	TFT.fillScreen(ST7735_BLACK);
	TFT.setTextColor(ST7735_WHITE);
	TFT.setTextSize(1);
	TFT.setCursor(7, 5);
	TFT.fillRect(0, 1, 159, 15, BAR_Blue);
	TFT.print("Temperature    Humidity");

	TFT.setCursor(5, 35);
	TFT.setTextSize(2);
	TFT.print(temperatureF);
	TFT.setTextSize(1);
	TFT.print(" F");

	TFT.setCursor(84, 35);
	TFT.setTextSize(2);
	TFT.print(humidity);
	TFT.setTextSize(1);
	TFT.print(" %");

	TFT.fillRect(0, 61, 159, 14, BAR_Blue);
	TFT.setCursor(55, 64);
	TFT.print("CO");
	TFT.setCursor(67, 67);
	TFT.print("2");
	TFT.setTextSize(1);
	TFT.setCursor(80, 64);
	TFT.print("Level");


	TFT.setCursor(25, 87);
	TFT.setTextSize(2);
	TFT.print(displayPPM);
    TFT.setTextSize(1);
    TFT.print(" ppm");

	counter = 0;

if (displayPPM<=350)
{
   TFT.fillRect(11, 111, barLength, 9, BAR_Blue);
}
 
if (displayPPM>350 && displayPPM<=1000)
{
   TFT.fillRect(11, 111, barLength, 9, BAR_Blue);
   TFT.fillRect(20, 111, barLength-10, 9, BAR_Green);
}

if (displayPPM>1000 && displayPPM <= 2000)
{
	TFT.fillRect(11, 111, barLength, 9, BAR_Blue);
	TFT.fillRect(20, 111, barLength - 10, 9, BAR_Green);
	TFT.fillRect(49, 111, barLength - 39, 9, BAR_Yellow);
}
if (displayPPM>2000 && displayPPM <= 5000)
{
	TFT.fillRect(11, 111, barLength, 9, BAR_Blue);
	TFT.fillRect(20, 111, barLength - 10, 9, BAR_Green);
	TFT.fillRect(49, 111, barLength - 39, 9, BAR_Yellow);
	TFT.fillRect(67, 111, barLength - 57, 9, BAR_Orange);
}
if (displayPPM>5000)
{
	TFT.fillRect(11, 111, barLength, 9, BAR_Blue);
	TFT.fillRect(20, 111, barLength - 10, 9, BAR_Green);
	TFT.fillRect(49, 111, barLength - 39, 9, BAR_Yellow);
	TFT.fillRect(67, 111, barLength - 57, 9, BAR_Orange);
	TFT.fillRect(93, 111, barLength - 83, 9, BAR_Red);
}
 
// temperature and humidity frame

TFT.drawFastHLine(0, 60, 160, ST7735_WHITE);
TFT.drawFastVLine(80, 0, 60, ST7735_WHITE);

// CO2 frame
TFT.drawFastHLine(0, 75, 160, ST7735_WHITE);


//bar frame

TFT.drawFastHLine(10, 110, 140, ST7735_WHITE);
TFT.drawFastVLine(10, 110, 10, ST7735_WHITE);
TFT.drawFastHLine(10, 120, 140, ST7735_WHITE);
TFT.drawFastVLine(149, 110, 10, ST7735_WHITE);

  //outer frame
  TFT.drawFastVLine(159, 0, 128, ST7735_WHITE);
  TFT.drawFastVLine(0, 0, 128, ST7735_WHITE);
  TFT.drawFastHLine(0, 0, 160, ST7735_WHITE);
  TFT.drawFastHLine(0, 15, 160, ST7735_WHITE);
  TFT.drawFastHLine(0, 127, 160, ST7735_WHITE);

  //ticks
  TFT.drawFastVLine(19, 121, 3, ST7735_WHITE); //350 mark
  TFT.drawFastVLine(48, 121, 3, ST7735_WHITE); //1000 mark
  TFT.drawFastVLine(67, 121, 3, ST7735_WHITE); //2000 mark
  TFT.drawFastVLine(93, 121, 3, ST7735_WHITE); //5000 mark

  //Trends
  switch (humidityTrend)
  {
  case 0:
	  TFT.setTextColor(ST7735_BLUE);
	  TFT.setCursor(83, 17);
	  TFT.setTextSize(2);
	  TFT.print("=");
	  break;
  case 1:
	  TFT.setTextColor(ST7735_GREEN);
	  TFT.setCursor(83, 17);
	  TFT.setTextSize(2);
	  TFT.print("\x18");
	  break;
  case 2:
	  TFT.setTextColor(ST7735_RED);
	  TFT.setCursor(83, 17);
	  TFT.setTextSize(2);
	  TFT.print("\x19");
	  break;
  }

  switch (tempTrend)
  {
  case 0:
	  TFT.setTextColor(ST7735_BLUE);
	  TFT.setCursor(2, 17);
	  TFT.setTextSize(2);
	  TFT.print("=");
	  break;
  case 1:
	  TFT.setTextColor(ST7735_GREEN);
	  TFT.setCursor(2, 17);
	  TFT.setTextSize(2);
	  TFT.print("\x18");
	  break;
  case 2:
	  TFT.setTextColor(ST7735_RED);
	  TFT.setCursor(2, 17);
	  TFT.setTextSize(2);
	  TFT.print("\x19");
	  break;
  }

  switch (ppmTrend)
  {
  case 0:
	  TFT.setTextColor(ST7735_BLUE);
	  TFT.setCursor(2, 77);
	  TFT.setTextSize(2);
	  TFT.print("=");
	  break;
  case 1:
	  TFT.setTextColor(ST7735_GREEN);
	  TFT.setCursor(2, 77);
	  TFT.setTextSize(2);
	  TFT.print("\x18");
	  break;
  case 2:
	  TFT.setTextColor(ST7735_RED);
	  TFT.setCursor(2, 77);
	  TFT.setTextSize(2);
	  TFT.print("\x19");
	  break;
  }

/*
  The following lines can be commented once the sensor is calibrated in open clear air environment.The rzero value should be entered
  in the MQ135.h file.Note that the sensor calibration tends to drift after the initial operation(a couple of weeks...), so it is recommended
  to keep this uncommented until the sensor reading stabilizes
*/
  TFT.setCursor(10, 65);
  TFT.setTextSize(1);
  TFT.print(rzero);  






}

