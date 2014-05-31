/*  Copyright (C) 2014  Adam Green (https://github.com/adamgreen)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/
#ifndef __FAKE_GPS_H__
#define __FAKE_GPS_H__

#include <mbed.h>
#include <stdint.h>

class FakeGPS
{
public:
    FakeGPS(PinName txPin, PinName rxPin, int baudRate) : m_serial(txPin, rxPin)
    {
        m_serial.baud(baudRate);
        m_ticker.attach(this, &FakeGPS::tickerISR, (1.0f / baudRate) * 100.0f);
    }

protected:
    void tickerISR();

    Ticker              m_ticker;
    Serial              m_serial;
};

#endif // __FAKE_GPS_H__
