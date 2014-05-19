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

#ifndef __CONFIG_H__
#define __CONFIG_H__


// --------------------------------------------------------------------------
// THIS IS THE trackMYmbed FIRMWARE CONFIGURATION FILE. YOUR CALLSIGN AND
// OTHER SETTINGS GO HERE.
// --------------------------------------------------------------------------


// --------------------------------------------------------------------------
// APRS config
// --------------------------------------------------------------------------

// Set your callsign and SSID here. Common values for the SSID are
// (from http://zlhams.wikidot.com/aprs-ssidguide):
//
// - Balloons:  11
// - Cars:       9
// - Home:       0
// - IGate:      5
#define S_CALLSIGN      "MYCALL"
#define S_CALLSIGN_ID   11

// Destination callsign: APRS (with SSID=0) is usually okay.
#define D_CALLSIGN      "APRS"
#define D_CALLSIGN_ID   0

// Digipeating paths:
// (read more about digipeating paths here: http://wa8lmf.net/DigiPaths/ )
// The recommended digi path for a balloon is WIDE2-1 or pathless. The default
// is pathless. Uncomment the following two lines for WIDE2-1 path:
#define DIGI_PATH1      "WIDE2"
#define DIGI_PATH1_TTL  1

// APRS comment: this goes in the comment portion of the APRS message. You
// might want to keep this short. The longer the packet, the more vulnerable
// it is to noise.
#define APRS_COMMENT    "Trackuino reminder: replace callsign with your own"


// --------------------------------------------------------------------------
// AX.25 config
// --------------------------------------------------------------------------

// TX delay in milliseconds
#define TX_DELAY      300

// --------------------------------------------------------------------------
// Tracker config
// --------------------------------------------------------------------------

// APRS packets are slotted so that multiple trackers can be used without
// them stepping on one another. The transmission times are governed by
// the formula:
//
//         APRS_SLOT (seconds) + n * APRS_PERIOD (seconds)
//
// When launching multiple balloons, use the same APRS_PERIOD in all balloons
// and set APRS_SLOT so that the packets are spaced equally in time.
// Eg. for two balloons and APRS_PERIOD = 60, set APRS_SLOT to 0 and 30,
// respectively. The first balloon will transmit at 00:00:00, 00:01:00,
// 00:02:00, etc. and the second balloon will transmit at 00:00:30, 00:01:30,
// 00:02:30, etc.
#define APRS_SLOT     -1    // seconds. -1 disables slotted transmissions
#define APRS_PERIOD   60    // seconds

// GPS baud rate (in bits per second). This is also the baud rate at which
// debug data will be printed out the serial port.
#define GPS_BAUDRATE  9600

// GPS_TX_PIN is the UART pin used for sending data to the GPS module.
#define GPS_TX_PIN      p9

// GPS_TX_PIN is the UART pin used for receiving data from the GPS module.
#define GPS_RX_PIN      p10

// Size of circular queue used to hold GPS sentences to be processed.  Must be
// a power of 2.
#define GPS_QUEUE_SIZE  256

// --------------------------------------------------------------------------
// Modem / AFSK config
// --------------------------------------------------------------------------

// Pre-emphasize the 2200 tone by 6 dB. This is actually done by
// de-emphasizing the 1200 tone by 6 dB and it might greatly improve
// reception at the expense of poorer FM deviation, which translates
// into an overall lower amplitude of the received signal. 1 = yes, 0 = no.
#define PRE_EMPHASIS    1

// --------------------------------------------------------------------------
// Radio config
// --------------------------------------------------------------------------

// This is the enable pin for the HX1 radio.
#define HX1_ENABLE_PIN  p20

// AUDIO_PIN is the audio-out pin.
#define AUDIO_PIN       p18

// --------------------------------------------------------------------------
// Debug config
// --------------------------------------------------------------------------

// Debug info includes printouts from different modules to aid in testing and
// debugging.
//
// Set macro to 1 for turning on that debug feature and 0 otherwise.
//
#define DEBUG_GPS   1   // GPS sentence dump and checksum validation
#define DEBUG_QUEUE 1   // Track max elements used in queue to see if possible to shrink.
#define DEBUG_AX25  1   // AX.25 frame dump
#define DEBUG_RESET 1   // Processor reset


#endif

