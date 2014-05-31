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
#include <mbed.h>

// Trackuino/trackMYmbed custom libs
#include "config.h"
#include "radio_hx1.h"
#include "radioout_mbed.h"
#include "aprs.h"
#include "gps.h"
#include "fakegps.h"

static Timer g_timer;

#if FAKE_GPS_ENABLE
    static FakeGPS g_fakeGPS(FAKE_GPS_TX_PIN, FAKE_GPS_RX_PIN, GPS_BAUDRATE);
#endif


// Forward Declarations
static void waitForPreviousSendToComplete(APRS* pAPRS);


// UNDONE: Will work on power saving once running on real hardware.
void powerSave(void)
{
}


int main(void)
{
    int32_t      nextAPRS;
    RadioOutMbed radioOut(AUDIO_PIN);
    RadioHx1     radio(&radioOut, HX1_ENABLE_PIN);
    APRS         aprs(&radio);
    GPS          gps(GPS_TX_PIN, GPS_RX_PIN);
    GPSData      gpsData;

    if (DEBUG_RESET)
        printf("RESET\r\n");

    memset(&gpsData, 0, sizeof(gpsData));
    gps.setup(GPS_BAUDRATE);

    // Do not start until we get a valid time reference
    // for slotted transmissions.
    if (APRS_SLOT >= 0)
    {
        while (!gps.decodeAvailableLines(&gpsData))
            powerSave();

        nextAPRS = 1000 * (APRS_PERIOD - (gpsData.seconds + APRS_PERIOD - APRS_SLOT) % APRS_PERIOD);
    }
    else
    {
        nextAPRS = APRS_PERIOD * 1000L;
    }
    g_timer.start();

    for (;;)
    {
        // Parse GPS lines as they become available.
        if (gps.hasLinesToDecode())
            gps.decodeAvailableLines(&gpsData);

        // Time to send another APRS frame?
        if (g_timer.read_ms() >= nextAPRS)
        {
            waitForPreviousSendToComplete(&aprs);
            aprs.send(&gpsData);
            g_timer.reset();
            nextAPRS = APRS_PERIOD * 1000L;
        }

        powerSave();
    }
}

static void waitForPreviousSendToComplete(APRS* pAPRS)
{
    while (!pAPRS->isSendComplete())
    {
    }
}
