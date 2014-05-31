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

#ifndef __AFSK_H__
#define __AFSK_H__

#include <assert.h>
#include <mbed.h>
#include "radio.h"
#include "radioout.h"

class AFSK
{
public:
    AFSK(IRadio* pRadio, uint32_t playbackRate = 38400) : m_pRadio(pRadio)
    {
        setRates(playbackRate);
        m_bitCount = 0;
        m_isSending = false;
    }

    void sendData(const void* pData, size_t bitCount)
    {
        // User shouldn't call while data is still being sent.
        assert ( isSendComplete() );

        m_pDataCurr = (const uint8_t*)pData;
        m_phaseDelta = m_phaseDelta1200;
        m_phase = 0;
        m_bitCount = bitCount;
        m_bitPos = 0;
        m_currentSampleInBaud = 0;
        m_isSending = true;
        m_pRadio->enable();
        m_ticker.attach_us(this, &AFSK::tickerISR, m_sampleInterval);
    }

    bool isSendComplete()
    {
        return !m_isSending;
    }

protected:
    void setRates(uint32_t playbackRate)
    {
        static const size_t tableSize = sizeof(s_sinLookupTable)/sizeof(s_sinLookupTable[0]);

        // mbed Ticker objects use microseconds for interval.
        m_sampleInterval = 1000000 / playbackRate;

        // The actual baudrate after rounding errors will be:
        // PLAYBACK_RATE / (integer_part_of((PLAYBACK_RATE * 256) / BAUD_RATE) / 256)

        // Samples per baud (bit) - Fixed point 24.8
        m_samplesPerBaud = (playbackRate << 8) / BAUD_RATE;

        // At 1200 and 2200 Hz, how many sin() table entries should be stepped over per sample?
        // Fixed point 25.7
        m_phaseDelta1200 = (((tableSize * 1200UL) << 7) / playbackRate);
        m_phaseDelta2200 = (((tableSize * 2200UL) << 7) / playbackRate);
        m_phaseDeltaSwitch = m_phaseDelta1200 ^ m_phaseDelta2200;
    }
    void tickerISR(void);

    Ticker                  m_ticker;
    IRadio*                 m_pRadio;
    volatile const uint8_t* m_pDataCurr;
    uint32_t                m_sampleInterval;
    uint32_t                m_samplesPerBaud;   // Fixed point 24.8
    uint32_t                m_phaseDelta1200;   // Fixed point 25.7
    uint32_t                m_phaseDelta2200;   // Fixed point 25.7
    uint32_t                m_phaseDeltaSwitch; // Fixed point 25.7
    volatile uint32_t       m_phaseDelta;       // Fixed point 25.7
    volatile uint32_t       m_phase;            // Fixed point 25.7
    volatile uint32_t       m_currentSampleInBaud;
    volatile uint32_t       m_bitCount;
    volatile uint32_t       m_bitPos;
    volatile uint8_t        m_currentByte;
    volatile bool           m_isSending;

    static const uint16_t   s_sinLookupTable[512];

    enum { BAUD_RATE = 1200 };
};

#endif // __AFSK_H__
