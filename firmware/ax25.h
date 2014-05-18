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

#ifndef __AX25_H__
#define __AX25_H__

#include "afsk.h"

struct AX25Address
{
	char          callsign[7];
	unsigned char ssid;
};

class AX25
{
public:
    AX25(IRadio* pRadio, uint32_t playbackRate = 38400) : m_afsk(pRadio, playbackRate)
    {
        m_packetSize = 0;
        m_frameOverflow = false;
    }

    void queueHeader(const AX25Address* pAddresses, int numAddresses);
    void queueByte(unsigned char byte);
    void queueString(const char* pString);
    void queueFooter();
    void sendFrame();
    bool isSendComplete()
    {
        return m_afsk.isSendComplete();
    }
    bool frameOverflowDetected()
    {
        return m_frameOverflow;
    }

protected:
    void updateCRC(uint8_t bit);
    void queueRawByte(uint8_t byte);
    void queueFlag();

    AFSK     m_afsk;
    uint32_t m_onesInARow;
    uint16_t m_crc;
    uint32_t m_packetSize;
    uint8_t  m_packet[512];
    bool     m_frameOverflow;
};


#endif
