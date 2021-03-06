  /*  
    main.cpp RF Amplifier and LPF controller.
    Copyright (C) 2021  Martyn Osborn 5B4AMO

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <band.h>

#define ONE_WIRE_BUS 2               // temp sensors onewire is plugged into port 2 on the Arduino
OneWire oneWire(ONE_WIRE_BUS);       // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // Pass oneWire reference to Dallas Temperature.

void HMI_read();
void BandSW_Reset(void);
void HMI_display_update(void);
void reset(void);
void PwrSwr_read(void);
void VoltCurrent_read(void);
void Temp_read(void);

const uint8_t term[3] = {0xff, 0xff, 0xff};

const unsigned int PwrSwrRefreshInterval = 100;      //Power and SWR fastest loop
const unsigned int VoltCurrentRefreshInterval = 250; // Voltage Current and CAT input next fastest loop
const unsigned int TempRefreshInterval = 1000;       // Temp is slow >500ms for temp read
unsigned long previousPwrSwrMillis = 0;
unsigned long previousVoltageCurrentMillis = 0;
unsigned long previousTempMillis = 0;

unsigned int power;
unsigned int swr;
unsigned int current;
unsigned int voltage;
unsigned int temp;

static uint8_t band_set = 0; //preserved between calls
static uint8_t auto_on = 0;  //preserved between calls

Band band; //connect to lib functions

void setup()
{
  //This is for the debug serial monitor.
  Serial.begin(57600);
  Serial.println("Martyn 5B4AMO HMI Test");
  //This is for serial port 1, which is the one used for the Nextion
  Serial1.begin(57600);
  //This is for serial port 2, which is the one used for CAT control
  Serial2.begin(57600);

  sensors.begin();                     //for temps on 1wire
  sensors.setWaitForConversion(false); //Async reading of temp sensors

  pinMode(A0, INPUT); // Power analogue
  pinMode(A1, INPUT); // SWR analogue
  pinMode(A2, INPUT); // Current analogue
  pinMode(A3, INPUT); // Voltage analogue

  reset(); //start on 160M, auto off
}

void loop()
{
  unsigned long currentTime = millis();
  HMI_read();

  //Note - this "else if" gives priority to swr & pwr. The other loops may not get done at right time. TBD use "if" instead of "else if"?
  if (currentTime - previousPwrSwrMillis >= PwrSwrRefreshInterval) // for the fast Power and SWR update
  {
    // unsigned long start = millis();
    PwrSwr_read();
    HMI_display_update();
    // unsigned long end = millis();
    // Serial.print("Read and display = ");
    // Serial.print(end-start);
    // Serial.println(" ms");
    previousPwrSwrMillis = currentTime;
  }
  else if (currentTime - previousVoltageCurrentMillis >= VoltCurrentRefreshInterval) // for the slower Voltage, Current and CAT update
  {
    VoltCurrent_read();

    if (auto_on)
    {
      band_set = band.Cat_band();
      if (band_set < 16)
      {
        band.SetBandBCD(band_set);
      }
      else
      {
        // TO DO: error handler for cat off, bad button code
      }
      HMI_display_update();
      // Serial.print("Band is = ");
      // Serial.println(band_set);
    }
    previousVoltageCurrentMillis = currentTime;
  }
  else if (currentTime - previousTempMillis >= TempRefreshInterval) // for the slowest Temp update
  {
    Temp_read();
    HMI_display_update();
    previousTempMillis = currentTime;
  }
}

void PwrSwr_read(void)
{
  unsigned int power_raw;
  power_raw = analogRead(A0);
  power = map(power_raw, 0, 1023, 0, 125); //full adc of 5V gives 125W
  //Serial.println(power);

  unsigned int swr_raw;
  swr_raw = analogRead(A1);
  swr = map(swr_raw, 0, 1023, 10, 35); //full adc of 5V gives SWR 3.5:1, 0V gives 1:1
  //Serial.println(swr);
}

void VoltCurrent_read(void)
{
  unsigned int current_raw;
  current_raw = analogRead(A2);
  current = map(current_raw, 0, 1023, 0, 250); //full adc of 5V gives 25.0A
  //Serial.println(current);

  unsigned int voltage_raw;
  voltage_raw = analogRead(A3);
  voltage = map(voltage_raw, 0, 1023, 0, 140); //full adc of 5V gives 14.0V
  //Serial.println(voltage);
}

void Temp_read(void)
{
  float temp_max;
  float tempC[3];
  temp_max = 0;
  tempC[0] = sensors.getTempCByIndex(0);
  tempC[1] = sensors.getTempCByIndex(1);
  tempC[2] = sensors.getTempCByIndex(2);
  for (int j = 0; j < 3; j++)
  {
    if (tempC[j] > temp_max)
    {
      temp_max = tempC[j];
    }
  }
  temp = round(temp_max * 10);   //0 to 999 or 99.9 degrees
  sensors.requestTemperatures(); // Send the command to get temperatures for next time round
  // Serial.print("Temp0 = ");
  // Serial.print(tempC[0]);
  // Serial.print("\t");
  // Serial.print("Temp1 = ");
  // Serial.print(tempC[1]);
  // Serial.print("\t");
  // Serial.print("Temp2 = ");
  // Serial.print(tempC[2]);
  // Serial.print("\t");
  // Serial.println(temp);
}

void HMI_read()
{
  uint8_t rxData[30];
  uint8_t data;
  uint8_t counter;
  uint8_t button;

  while (Serial1.available() > 0)
  {
    data = Serial1.read();
    rxData[counter] = data;
    if ((counter > 3) && (rxData[counter] == 0xff) && (rxData[counter - 1] == 0xff) && (rxData[counter - 2] == 0xff))
    {
      // Touch event -6(0x65); -5(pg 0x00), -4(id), -3(0x00 release)
      if ((rxData[counter - 6] == 0x65) && (rxData[counter - 5] == 0x00) && (rxData[counter - 3] == 0x00))
      {
        button = rxData[counter - 4];
        // Serial.print("Button pressed is ");
        // Serial.println(button);
        band_set = band.process_button(button, auto_on);
        if (band_set < 16)
        {
          band.SetBandBCD(band_set);
        }
        else
        {
          // error handle for cat off, bad button code
        }
      }
      counter = 0;
    }
    counter++;
    if (counter >= 30)
    { //if too much data in...
      counter = 0;
    }
  }
}

void reset(void)
{
  band_set = 160;
  band.SetBandBCD(band_set);
  auto_on = 0;
  power = 0;
  swr = 0;
  current = 0;
  voltage = 0;
  temp = 0;

  for (uint8_t j = 100; j >= 1; j -= 2)
  {
    Serial1.print(F("j0.val="));
    Serial1.print(j);
    Serial1.write(term, 3);

    Serial1.print(F("j1.val="));
    Serial1.print(j);
    Serial1.write(term, 3);

    Serial1.print(F("j2.val="));
    Serial1.print(j);
    Serial1.write(term, 3);

    Serial1.print(F("j3.val="));
    Serial1.print(j);
    Serial1.write(term, 3);

    Serial1.print(F("j4.val="));
    Serial1.print(j);
    Serial1.write(term, 3);

    delay(15);
  }
  HMI_display_update();
}

void HMI_display_update(void)
{
  if (power > 99)
  {
    Serial1.print(F("x0.vvs0=3")); //if power is over 99, display 3 places
    Serial1.write(term, 3);
  }
  else
  {
    Serial1.print(F("x0.vvs0=2"));
    Serial1.write(term, 3);
  }
  Serial1.print(F("x0.val=")); //Power 0-125W
  Serial1.print(power);
  Serial1.write(term, 3);
  Serial1.print(F("j2.val=")); //Slider Power
  Serial1.print(map(power, 0, 125, 0, 100));
  Serial1.write(term, 3);

  Serial1.print(F("x1.val=")); //SWR 10 is 1:1; 35 is 3.5:1
  Serial1.print(swr);
  Serial1.write(term, 3);
  Serial1.print(F("j3.val=")); //Slider SWR
  Serial1.print(map(swr, 10, 35, 5, 100));
  Serial1.write(term, 3);

  Serial1.print(F("x2.val=")); //Current 0 is 0A; 250 is 25.0A
  Serial1.print(current);
  Serial1.write(term, 3);
  Serial1.print(F("j4.val=")); //Slider Current
  Serial1.print(map(current, 0, 250, 0, 100));
  Serial1.write(term, 3);

  if (temp < 500)
  {
    Serial1.print(F("j0.pco=31")); //if temp is less than 50deg, display in blue
    Serial1.write(term, 3);
  }
  else
  {
    Serial1.print(F("j0.pco=63488")); // over 50deg bar is red
    Serial1.write(term, 3);
  }
  Serial1.print(F("x3.val=")); //Temp 0 is 0deg; 999 is 99.9deg
  Serial1.print(temp);
  Serial1.write(term, 3);
  Serial1.print(F("j0.val=")); //Slider Temp
  Serial1.print(map(temp, 0, 999, 0, 100));
  Serial1.write(term, 3);

  if (voltage < 138)
  {
    Serial1.print(F("j1.pco=31")); //if volts less than 13.8, display in blue
    Serial1.write(term, 3);
  }
  else
  {
    Serial1.print(F("j1.pco=63488")); // over 13.8 bar is red
    Serial1.write(term, 3);
  }
  Serial1.print(F("x4.val=")); //Voltage 0 =0V; 140 =14.0V
  Serial1.print(voltage);
  Serial1.write(term, 3);
  int volt_bar;
  volt_bar = voltage - 120; //adjust bar for 12.0 to 14.0V
  if (volt_bar <= 0)
    volt_bar = 0;
  Serial1.print(F("j1.val=")); //Slider Voltage
  Serial1.print(map(volt_bar, 0, 20, 0, 100));
  Serial1.write(term, 3);
}
