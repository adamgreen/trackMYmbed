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
#include <assert.h>
#include <stdio.h>
#include "gps.h"
#include "interlock.h"


#define ARRAY_SIZE(X) sizeof(X)/sizeof(X[0])

typedef void (*nmeaParser)(GPS* pThis, const char *token);

unsigned char fromHex(char a);


void GPS::resetForNewLine()
{
      m_atChecksum = false;
      m_ourChecksum = '$';
      m_theirChecksum = 0;
      m_offset = 0;
      m_numTokens = 0;
      m_sentenceType = SENTENCE_UNK;
}

void GPS::serialRxISR()
{
    while (m_serial.readable())
    {
        char c = m_serial.getc();
        // End the current line with a NULL terminator if this is the first line terminator seen since the start of
        // the line.  Ignore any line terminators until after starting the next line of text.  Also never copy a
        // NULL terminator from the serial port to the queue since it will look like a line terminator and isn't a valid
        // NMEA character either.
        if (c == '\r' || c == '\n')
        {
            if (m_lastChar != '\r' && m_lastChar != '\n')
            {
                // Only increment line counter if successfully queued up line terminator.
                if (m_queue.enqueue('\0'))
                    interlockedIncrement(&m_lineCount);
            }
        }
        else if (c != '\0')
        {
            m_queue.enqueue(c);
        }
        m_lastChar = c;
    }
}

bool GPS::decodeAvailableLines(GPSData* pData)
{
    bool     ret = false;
    uint32_t lineCount = m_lineCount;

    if (DEBUG_QUEUE)
    {
        uint32_t droppedElements = m_queue.droppedElementCount();
        if (droppedElements > 0)
            printf("\r\nDropped %lu elements from the GPS data queue.\r\n", droppedElements);
    }

    // Process each of the available lines.
    while (lineCount--)
    {
        char c;

        // Process this line of text.
        do
        {
            // We should only hit an empty queue before the NULL terminator if the queue overflowed.
            if (!m_queue.hasData())
            {
                if (DEBUG_GPS)
                    printf("\r\nGPS data queue was missing NULL terminator\r\n");
                resetForNewLine();
                break;
            }

            c = m_queue.dequeue();
        } while (!decode(c));

        // Valid position scenario:
        //
        // 1. The timestamps of the two previous GGA/RMC sentences must match.
        //
        // 2. We just processed a known (GGA/RMC) sentence. Suppose the
        //    contrary: after starting up this module, gga_time and rmc_time
        //    are both equal (they're both initialized to ""), so (1) holds
        //    and we wrongly report a valid position.
        //
        // 3. The GPS has a valid fix. For some reason, the Venus 634FLPX
        //    reports 24 deg N, 121 deg E (the middle of Taiwan) until a valid
        //    fix is acquired:
        //
        //    $GPGGA,120003.000,2400.0000,N,12100.0000,E,0,00,0.0,0.0,M,0.0,M,,0000*69 (OK!)
        //    $GPGSA,A,1,,,,,,,,,,,,,0.0,0.0,0.0*30 (OK!)
        //    $GPRMC,120003.000,V,2400.0000,N,12100.0000,E,000.0,000.0,280606,,,N*78 (OK!)
        //    $GPVTG,000.0,T,,M,000.0,N,000.0,K,N*02 (OK!)
        if (strcmp(m_ggaTime, m_rmcTime) == 0 && m_active)
        {
            // Atomically merge data from the two sentences
            strcpy(pData->time, m_newTime);
            pData->seconds = m_newSeconds;
            pData->latitude = m_newLatitude;
            pData->longitude = m_newLongitude;
            strcpy(pData->aprsLatitude, m_newAprsLatitude);
            strcpy(pData->aprsLongitude, m_newAprsLongitude);
            pData->course = m_newCourse;
            pData->speed = m_newSpeed;
            pData->altitude = m_newAltitude;
            ret = true;
        }

        interlockedDecrement(&m_lineCount);
    }

    return ret;
}

bool GPS::decode(char c)
{
    static const nmeaParser unkParsers[] =
    {
        parseSentenceType,    // $GPxxx
    };

    static const nmeaParser ggaParsers[] =
    {
        NULL,             // $GPGGA
        parseTime,        // Time
        NULL,             // Latitude
        NULL,             // N/S
        NULL,             // Longitude
        NULL,             // E/W
        NULL,             // Fix quality
        NULL,             // Number of satellites
        NULL,             // Horizontal dilution of position
        parseAltitude,    // Altitude
        NULL,             // "M" (mean sea level)
        NULL,             // Height of GEOID (MSL) above WGS84 ellipsoid
        NULL,             // "M" (mean sea level)
        NULL,             // Time in seconds since the last DGPS update
        NULL              // DGPS station ID number
    };

    static const nmeaParser rmcParsers[] =
    {
        NULL,                   // $GPRMC
        parseTime,              // Time
        parseStatus,            // A=active, V=void
        parseLatitude,          // Latitude,
        parseLatitudeHemi,      // N/S
        parseLongitude,         // Longitude
        parseLongitudeHemi,     // E/W
        parseSpeed,             // Speed over ground in knots
        parseCourse,            // Track angle in degrees (true)
        NULL,                   // Date (DDMMYY)
        NULL,                   // Magnetic variation
        NULL                    // E/W
    };
    bool ret = false;

    switch(c)
    {
    case '\0':
        // End of sentence
        if (m_numTokens && m_ourChecksum == m_theirChecksum)
        {
            if (DEBUG_GPS)
            {
                printf(" (OK!) ");
                // UNDONE: Serial.print(millis());
            }

            // Return a valid position only when we've got two rmc and gga
            // messages with the same timestamp.
            switch (m_sentenceType)
            {
            case SENTENCE_UNK:
                break;
            case SENTENCE_GGA:
                strcpy(m_ggaTime, m_newTime);
                break;
            case SENTENCE_RMC:
                strcpy(m_rmcTime, m_newTime);
                break;
            }
        }
        if (DEBUG_GPS && m_numTokens)
                printf("\r\n");
        resetForNewLine();
        ret = true;
        break;

    case '*':
        m_atChecksum = true;
        m_ourChecksum ^= c;
        // Fall Through!  Handle as ',', but prepares to receive checksum

    case ',':
        // Process token
        m_token[m_offset] = '\0';
        // Checksum the ',', undo the '*' if we fell through.
        m_ourChecksum ^= c;

        // Parse token
        switch (m_sentenceType)
        {
        case SENTENCE_UNK:
            if (m_numTokens < ARRAY_SIZE(unkParsers) && unkParsers[m_numTokens])
                unkParsers[m_numTokens](this, m_token);
            break;
        case SENTENCE_GGA:
            if (m_numTokens < ARRAY_SIZE(ggaParsers) && ggaParsers[m_numTokens])
                ggaParsers[m_numTokens](this, m_token);
            break;
        case SENTENCE_RMC:
            if (m_numTokens < ARRAY_SIZE(rmcParsers) && rmcParsers[m_numTokens])
                rmcParsers[m_numTokens](this, m_token);
            break;
        }

        // Prepare for next token
        m_numTokens++;
        m_offset = 0;

        if (DEBUG_GPS)
            printf("%c", c);
        break;

    default:
        // Any other character
        if (m_atChecksum)
        {
            // Checksum value
            m_theirChecksum = m_theirChecksum * 16 + fromHex(c);
        }
        else
        {
            // Regular NMEA data
            // Avoid buffer overrun (tokens can't be > 15 chars)
            // Leave room for NULL terminator.
            if (m_offset < ARRAY_SIZE(m_token) - 1)
            {
                m_token[m_offset] = c;
                m_offset++;
                m_ourChecksum ^= c;
            }
        }
        if (DEBUG_GPS)
            printf("%c", c);
    }
    return ret;
}

// Static token parsing methods.
void GPS::parseSentenceType(GPS* pThis, const char * token)
{
    if (strcmp(pThis->m_token, "$GPGGA") == 0)
        pThis->m_sentenceType = SENTENCE_GGA;
    else if (strcmp(pThis->m_token, "$GPRMC") == 0)
        pThis->m_sentenceType = SENTENCE_RMC;
    else
        pThis->m_sentenceType = SENTENCE_UNK;
}

void GPS::parseTime(GPS* pThis, const char *token)
{
    // Time can have decimals (fractions of a second), but we only take HHMMSS
    strncpy(pThis->m_newTime, token, 6);
    // Terminate string
    pThis->m_newTime[6] = '\0';

    pThis->m_newSeconds =
        ((pThis->m_newTime[0] - '0') * 10 + (pThis->m_newTime[1] - '0')) * 60 * 60UL +
        ((pThis->m_newTime[2] - '0') * 10 + (pThis->m_newTime[3] - '0')) * 60 +
        ((pThis->m_newTime[4] - '0') * 10 + (pThis->m_newTime[5] - '0'));
}

void GPS::parseStatus(GPS* pThis, const char *token)
{
    // "A" = active, "V" = void. We shoud disregard void sentences
    if (strcmp(token, "A") == 0)
        pThis->m_active = true;
    else
        pThis->m_active = false;
}

void GPS::parseLatitude(GPS* pThis, const char *token)
{
    // Parses latitude in the format "DD" + "MM" (+ ".M{...}M")
    char degs[3];
    if (strlen(token) >= 4)
    {
        degs[0] = token[0];
        degs[1] = token[1];
        degs[2] = '\0';
        pThis->m_newLatitude = atof(degs) + atof(token + 2) / 60.0f;
    }
    // APRS-ready latitude
    strncpy(pThis->m_newAprsLatitude, token, 7);
    pThis->m_newAprsLatitude[7] = '\0';
}

void GPS::parseLatitudeHemi(GPS* pThis, const char *token)
{
    if (token[0] == 'S')
        pThis->m_newLatitude *= -1;
    pThis->m_newAprsLatitude[7] = token[0];
    pThis->m_newAprsLatitude[8] = '\0';
}

void GPS::parseLongitude(GPS* pThis, const char *token)
{
    // Longitude is in the format "DDD" + "MM" (+ ".M{...}M")
    char degs[4];
    if (strlen(token) >= 5)
    {
        degs[0] = token[0];
        degs[1] = token[1];
        degs[2] = token[2];
        degs[3] = '\0';
        pThis->m_newLongitude = atof(degs) + atof(token + 3) / 60.0f;
    }
    // APRS-ready longitude
    strncpy(pThis->m_newAprsLongitude, token, 8);
    pThis->m_newAprsLongitude[8] = '\0';
}

void GPS::parseLongitudeHemi(GPS* pThis, const char *token)
{
    if (token[0] == 'W')
        pThis->m_newLongitude *= -1;
    pThis->m_newAprsLongitude[8] = token[0];
    pThis->m_newAprsLongitude[9] = '\0';
}

void GPS::parseSpeed(GPS* pThis, const char *token)
{
    pThis->m_newSpeed = atof(token);
}

void GPS::parseCourse(GPS* pThis, const char *token)
{
    pThis->m_newCourse = atof(token);
}

void GPS::parseAltitude(GPS* pThis, const char *token)
{
    pThis->m_newAltitude = atof(token);
}

unsigned char fromHex(char a)
{
    if (a >= 'A' && a <= 'F')
        return a - 'A' + 10;
    else if (a >= 'a' && a <= 'f')
        return a - 'a' + 10;
    else if (a >= '0' && a <= '9')
        return a - '0';
    else
        return 0;
}
