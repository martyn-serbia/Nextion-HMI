/*
  band.cpp - Library to set up band select.
  RF Amplifier and LPF controller.
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

#include "Arduino.h"
#include "band.h"

int Band::SetBandBCD(uint8_t band_no) //this is mega 2650 sepcific
{
  DDRA = 0xff; //set A0 to A7 as o/p
  uint8_t highBandNib = (0xF0 & band_no) >> 4;
  uint8_t loBandNib = 0x0F & band_no;
  PORTA = loBandNib; //LSB; PA0; D22 - MSB PA3 D25
  if (highBandNib)
  {
    return 0;
  }
  return 1;
}

int Band::Cat_band(void)
{
  const char *CatFrqReq[6] = {"FA;", "FB;", "FC;", "FD;", "FE;", "FF;"};
  long CatFreq[6];
  if (Serial2) //only if CAT connection is up
  {
    for (int i = 0; i < 6; i++)
    {
      Serial2.print(CatFrqReq[i]);
      CatFreq[i] = Serial2.parseInt();
      // Serial.print("Command = ");
      // Serial.print(CatFrqReq[i]);
      // Serial.print("/tResponse is:");
      // Serial.println(CatFreq[i]);
    }
    Serial2.print("FR;");
    int RX_index = Serial2.parseInt(); //get active reciever number
    long Frequency = CatFreq[RX_index];
    // Serial.print("RX_index = ");
    // Serial.print(RX_index);
    // Serial.print("/tFrequency is:  ");
    // Serial.println(Frequency);
    if (Frequency >= 1800000 && Frequency <= 2000000)
    {
      return 0; //160m = BCD 0
    }
    else if (Frequency >= 3500000 && Frequency <= 4000000)
    {
      return 1; //80m = BCD 1
    }
    else if (Frequency >= 5000000 && Frequency <= 5500000)
    {
      return 2; //60m = BCD 2 - no specific 60m lpf, 40m used
    }
    else if (Frequency >= 7000000 && Frequency <= 7300000)
    {
      return 2; //40m = BCD 2
    }
    else if (Frequency >= 10000000 && Frequency <= 10300000)
    {
      return 3; //30m = BCD 3
    }
    else if (Frequency >= 14000000 && Frequency <= 14350000)
    {
      return 4; //20m = BCD 4
    }
    else if (Frequency >= 18000000 && Frequency <= 18170000)
    {
      return 5; //17m = BCD 5
    }
    else if (Frequency >= 21000000 && Frequency <= 21450000)
    {
      return 6; //15m = BCD 6
    }
    else if (Frequency >= 24890000 && Frequency <= 25000000)
    {
      return 7; //12m = BCD 7
    }
    else if (Frequency >= 28000000 && Frequency <= 30000000)
    {
      return 8; //10m = BCD 8
    }
    else if (Frequency >= 50000000 && Frequency <= 52000000)
    {
      return 9; //6m = BCD 9
    }
    else if (Frequency >= 144000000 && Frequency <= 146000000)
    {
      return 0; //2m not implemented
    }
    else if (Frequency >= 430000000 && Frequency <= 440000000)
    {
      return 0; //70cm not implmented
    }
    else
    {
      return 0; //out of band, set 160M
    }
  }
  else
  {
    return 254; // Cat switched off, error code
  }
}

int Band::process_button(uint8_t press, uint8_t autof)
{
  const uint8_t term[3] = {0xff, 0xff, 0xff};
  switch (press)
  {
  case 2: //Toggle Auto
    autof = !autof;
    if (autof)
    {
      Serial1.print(F("bt7.val=1"));
      Serial1.write(term, 3);
    }
    else
    {
      Serial1.print(F("bt7.val=0"));
      Serial1.write(term, 3);
    }
    break;
  case 3: //160
    if (!autof)
    {
      Serial1.print(F("click b0,1"));
      Serial1.write(term, 3);
      return 0; //160m = BCD 0
    }
    break;
  case 4: //80
    if (!autof)
    {
      Serial1.print(F("click b1,1"));
      Serial1.write(term, 3);
      return 1; //80m = BCD 1
    }
    break;
  case 5: //40
    if (!autof)
    {
      Serial1.print(F("click b2,1"));
      Serial1.write(term, 3);
      return 2; //40m = BCD 2
    }
    break;
  case 6: //20
    if (!autof)
    {
      Serial1.print(F("click b3,1"));
      Serial1.write(term, 3);
      return 4; //20m = BCD 4
    }
    break;
  case 7: //15
    if (!autof)
    {
      Serial1.print(F("click b4,1"));
      Serial1.write(term, 3);
      return 6; //15m = BCD 6
    }
    break;
  case 8: //10
    if (!autof)
    {
      Serial1.print(F("click b5,1"));
      Serial1.write(term, 3);
      return 8; //10m = BCD 8
    }
    break;
  case 9: //6
    if (!autof)
    {
      Serial1.print(F("click b6,1"));
      Serial1.write(term, 3);
      return 9; //6m = BCD 9
    }
    break;
  case 22:
    return 255; // Reset has been pressed band_no = 255
    break;
  }
return 254; // default is ignore; return error code 254 button not known
}
