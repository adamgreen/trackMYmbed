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

#include <mbed.h>
#include <stdint.h>
#include "queue.h"

struct GPSData
{
    float    latitude;
    float    longitude;
    float    course;
    float    speed;
    float    altitude;
    // seconds after midnight
    uint32_t seconds;
    // HHMMSS
    char     time[7];
    // DDMMYY
    char      date[7];
    char      aprsLatitude[9];
    char      aprsLongitude[10];
};


class GPS
{
public:
    GPS(PinName txPin, PinName rxPin) : m_serial(txPin, rxPin)
    {
        m_ggaTime[0] = '\0';
        m_rmcTime[0] = '\0';
        m_lineCount = 0;
        m_lastChar = 0;
        m_active = false;
    }

    void setup(int baudRate)
    {
        m_serial.baud(baudRate);
        resetForNewLine();
        m_serial.attach(this, &GPS::serialRxISR);
    }

    bool decodeAvailableLines(GPSData* pData);


protected:
    void resetForNewLine();
    void serialRxISR();
    bool decode(char c, GPSData* pData);
    static void parseSentenceType(GPS* pThis, const char * token);
    static void parseTime(GPS* pThis, const char *token);
    static void parseStatus(GPS* pThis, const char *token);
    static void parseLatitude(GPS* pThis, const char *token);
    static void parseLatitudeHemi(GPS* pThis, const char *token);
    static void parseLongitude(GPS* pThis, const char *token);
    static void parseLongitudeHemi(GPS* pThis, const char *token);
    static void parseSpeed(GPS* pThis, const char *token);
    static void parseCourse(GPS* pThis, const char *token);
    static void parseAltitude(GPS* pThis, const char *token);

    enum SentenceType
    {
        SENTENCE_UNK,
        SENTENCE_GGA,
        SENTENCE_RMC
    };

    Serial              m_serial;
    Queue               m_queue;
    float               m_newLatitude;
    float               m_newLongitude;
    float               m_newCourse;
    float               m_newSpeed;
    float               m_newAltitude;
    volatile uint32_t   m_lineCount;
    uint32_t            m_offset;
    uint32_t            m_newSeconds;
    uint32_t            m_numTokens;
    SentenceType        m_sentenceType;
    bool                m_atChecksum;
    bool                m_active;
    uint8_t             m_ourChecksum;
    uint8_t             m_theirChecksum;
    char                m_lastChar;
    char                m_token[16];
    char                m_ggaTime[7];
    char                m_rmcTime[7];
    char                m_newTime[7];
    char                m_newAprsLatitude[9];
    char                m_newAprsLongitude[10];
};

#endif
