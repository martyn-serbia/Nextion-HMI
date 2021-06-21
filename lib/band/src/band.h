/*
  band.h - Library to set up band select.
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