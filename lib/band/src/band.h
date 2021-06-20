/*
  band.h - Library to set up band select.
  Created by Martyn 5B4AMO.
  A domain where I have dominion.
*/
#ifndef band_h
#define band_h

#include "Arduino.h"

extern const uint8_t term[3];
extern uint8_t auto_on;

class Band
{
  public:
    int SetBandBCD(uint8_t band_no);
    int Cat_band(void);
    int process_button(uint8_t press);

  private:
    
};

#endif