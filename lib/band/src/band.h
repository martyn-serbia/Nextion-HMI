/*
  band.h - Library to set up band select.
  Created by Martyn 5B4AMO.
  A domain where I have dominion.
*/
#ifndef band_h
#define band_h

#include "Arduino.h"


class Band
{
  public:
    int SetBandBCD(uint8_t band_no);
    int Cat_band(void);
    int process_button(uint8_t press, uint8_t autof);

  private:
    
};

#endif