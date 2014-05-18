/* trackuino copyright (C) 2010  EA5HAV Javi
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
#include "aprs.h"
#include "config.h"

static float metersToFeet(float m);

void APRS::send(const GPSData* pGPS)
{
    const struct AX25Address addresses[] =
    {
        // Destination callsign
        {D_CALLSIGN, D_CALLSIGN_ID},
        // Source callsign (-11 = balloon, -9 = car)
        {S_CALLSIGN, S_CALLSIGN_ID},
#ifdef DIGI_PATH1
        // Digi1 (first digi in the chain)
        {DIGI_PATH1, DIGI_PATH1_TTL},
#endif
#ifdef DIGI_PATH2
        // Digi2 (second digi in the chain)
        {DIGI_PATH2, DIGI_PATH2_TTL},
#endif
    };
    char temp[12];

    queueHeader(addresses, sizeof(addresses)/sizeof(addresses[0]));
    // Report w/ timestamp, no APRS messaging. $ = NMEA raw data
    queueByte('/');
    // queueString("021709z");     // 021709z = 2nd day of the month, 17:09 zulu (UTC/GMT)
    // 170915 = 17h:09m:15s zulu (not allowed in Status Reports)
    queueString(pGPS->time);
    queueByte('h');
    // Lat: 38deg and 22.20 min (.20 are NOT seconds, but 1/100th of minutes)
    queueString(pGPS->aprs_lat);
    // Symbol table
    queueByte('/');
    // Lon: 000deg and 25.80 min
    queueString(pGPS->aprs_lon);
    // Symbol: O=balloon, -=QTH
    queueByte('O');
    // Course (degrees)
    snprintf(temp, 4, "%03d", (int)(pGPS->course + 0.5));
    queueString(temp);
    // and
    queueByte('/');
    // speed (knots)
    snprintf(temp, 4, "%03d", (int)(pGPS->speed + 0.5));
    queueString(temp);
    // Altitude (feet). Goes anywhere in the comment area
    queueString("/A=");
    snprintf(temp, 7, "%06d", (int)(metersToFeet(pGPS->altitude) + 0.5));
    queueString(temp);

    queueString("/Ti=");
    snprintf(temp, 6, "%d", 0);
    queueString(temp);
    queueString("/Te=");
    snprintf(temp, 6, "%d", 0);
    queueString(temp);
    queueString("/V=");
    snprintf(temp, 6, "%d", 0);
    queueString(temp);

    queueByte(' ');
    // Comment
    queueString(APRS_COMMENT);
    queueFooter();

    sendFrame();
}

static float metersToFeet(float m)
{
  // 10000 ft = 3048 m
  return m / 0.3048f;
}
