/* trackuino copyright (C) 2010  EA5HAV Javi
 *           copyright (C) 2014         Adam Green (https://github.com/adamgreen)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __GPS_H__
#define __GPS_H__

struct GPSData
{
    float    lat;
    float    lon;
    float    course;
    float    speed;
    float    altitude;
    // seconds after midnight
    uint32_t seconds;
    // HHMMSS
    char     time[7];
    // DDMMYY
    char      date[7];
    char      aprs_lat[9];
    char      aprs_lon[10];
};

class GPS
{
public:
    GPS(PinName txPin, PinName rxPin) : m_serial(txPin, rxPin)
    {
    }

    void setup(int baudRate)
    {
        m_serial.baud(baudRate);
    }

    int available()
    {
        return 1;
    }

    int readAndDecode()
    {
        return 1;
    }

    uint32_t seconds()
    {
        return 0;
    }
    float altitude()
    {
        return 0.0f;
    }
protected:
    Serial m_serial;
};

#ifdef UNDONE
#include <stdint.h>

extern char gps_time[7];       // HHMMSS
extern uint32_t gps_seconds;   // seconds after midnight
extern char gps_date[7];       // DDMMYY
extern float gps_lat;
extern float gps_lon;
extern char gps_aprs_lat[9];
extern char gps_aprs_lon[10];
extern float gps_course;
extern float gps_speed;
extern float gps_altitude;

void gps_setup();
bool gps_decode(char c);
#endif // UNDONE

#endif
