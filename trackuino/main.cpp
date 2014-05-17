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
#include <mbed.h>

// Trackuino custom libs
#include "config.h"
#include "afsk.h"
#include "aprs.h"
#include "buzzer.h"
#include "gps.h"

// GPS timeout in milliseconds.
#define VALID_POS_TIMEOUT 2000

// Module variables
static int32_t    g_nextAPRS;
static Timer      g_timer;
static DigitalOut g_led(LED_PIN);
static GPS        g_gps(GPS_TX_PIN, GPS_RX_PIN);


// STUB:
void power_save(void)
{
}

void buzzer_setup()
{
}

void afsk_setup()
{
}

bool afsk_flush()
{
    return false;
}

void afsk_debug()
{
}

void buzzer_on()
{
}

void buzzer_off()
{
}

void aprs_send()
{
}

void setup()
{
    if (DEBUG_RESET)
        printf("RESET\r\n");

    g_timer.start();
    buzzer_setup();
    afsk_setup();
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

    if (valid_pos)
    {
        if (g_gps.altitude() > BUZZER_ALTITUDE)
            buzzer_off();   // In space, no one can hear you buzz
        else
            buzzer_on();
    }
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
        while (afsk_flush())
            power_save();

        if (DEBUG_MODEM)
            // Show modem ISR stats from the previous transmission
            afsk_debug();
    }
    power_save(); // Incoming GPS data or interrupts will wake us up
}

int main(void)
{
    setup();
    for (;;)
        loop();
}
