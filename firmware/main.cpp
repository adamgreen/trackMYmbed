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

// GPS timeout in milliseconds.
// Move to config.h
#define VALID_POS_TIMEOUT 2000

// Module variables
static int32_t      g_nextAPRS;
static Timer        g_timer;
static RadioOutMbed g_radioOut(AUDIO_PIN);
static RadioHx1     g_radio(&g_radioOut, HX1_ENABLE_PIN);
static APRS         g_aprs(&g_radio);
static GPS          g_gps(GPS_TX_PIN, GPS_RX_PIN);


// STUB:
void power_save(void)
{
}

void aprs_send()
{
}

void setup()
{
    GPSData gpsData;

    if (DEBUG_RESET)
        printf("RESET\r\n");

    // UNDONE: Just manually testing APRS for now.
    memset(&gpsData, 0, sizeof(gpsData));
    for (;;)
    {
        g_aprs.send(&gpsData);
        while (!g_aprs.isSendComplete())
        {
        }
        wait(1.0f);
    }

    g_timer.start();
    g_gps.setup(GPS_BAUDRATE);

    // Do not start until we get a valid time reference
    // for slotted transmissions.
    if (APRS_SLOT >= 0)
    {
        do
        {
            while (!g_gps.available())
                power_save();
        } while (!g_gps.readAndDecode());

        g_nextAPRS = 1000 * (APRS_PERIOD - (g_gps.seconds() + APRS_PERIOD - APRS_SLOT) % APRS_PERIOD);
    }
    else
    {
        g_nextAPRS = 0;
    }

    g_timer.reset();
}

static void get_pos()
{
    // Get a valid position from the GPS
    int valid_pos = 0;
    int timeout = g_timer.read_ms();
    do
    {
        if (g_gps.available())
            valid_pos = g_gps.readAndDecode();
    } while ( (g_timer.read_ms() - timeout < VALID_POS_TIMEOUT) && ! valid_pos) ;
}

void loop()
{
    // Time for another APRS frame
    if (g_timer.read_ms() >= g_nextAPRS)
    {
        get_pos();
        aprs_send();
        g_timer.reset();
        g_nextAPRS = APRS_PERIOD * 1000L;
    }
    power_save(); // Incoming GPS data or interrupts will wake us up
}

int main(void)
{
    setup();
    for (;;)
        loop();
}
