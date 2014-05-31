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
#include "fakegps.h"


void FakeGPS::tickerISR()
{
    // Test sentences were taken from http://www.satsleuth.com/gps_nmea_sentences.htm
    static const char testData[] = "$GPGGA,092204.999,4250.5589,S,14718.5084,E,1,04,24.4,19.7,M,,,,0000*1F\n"
                                   "$GPRMC,092204.999,A,4250.5589,S,14718.5084,E,0.00,89.68,211200,,*25\n";
    static const char* pCurr = testData;

    m_serial.putc(*pCurr);
    pCurr++;
    if ((size_t)(pCurr - testData) >= (sizeof(testData) - 1))
        pCurr = testData;
}
