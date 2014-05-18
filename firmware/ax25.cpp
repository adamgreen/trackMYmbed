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
#include "ax25.h"
#include "config.h"


void AX25::queueHeader(const AX25Address* pAddresses, int numAddresses)
{
    int i;
    int j;

    m_packetSize = 0;
    m_frameOverflow = false;
    m_onesInARow = 0;
    m_crc = 0xffff;

    // Send flags during TX_DELAY milliseconds
    //  8 bit-flag = 8 bits/flag * 1/1200 secs/bit * 1000 ms/sec = 8000/1200 ms/flag
    //  TX_DELAY / (8000/1200) = TX_DELAY * 1200 / 8000 = TX_DELAY * 3 / 20
    for (i = 0; i < TX_DELAY * 3 / 20; i++)
        queueFlag();

    for (i = 0; i < numAddresses; i++)
    {
        // Transmit callsign.
        for (j = 0; pAddresses[i].callsign[j] ; j++)
            queueRawByte(pAddresses[i].callsign[j] << 1);
        // Pad callsign with spaces to a width of 6.
        for ( ; j < 6; j++)
            queueRawByte(' ' << 1);
        // Transmit SSID. Termination signaled with last bit = 1
        if (i == numAddresses - 1)
            queueRawByte(('0' + pAddresses[i].ssid) << 1 | 1);
        else
            queueRawByte(('0' + pAddresses[i].ssid) << 1);
    }

    // Control field: 3 = APRS-UI frame
    queueRawByte(0x03);

    // Protocol ID: 0xf0 = no layer 3 data
    queueRawByte(0xf0);

    if (DEBUG_AX25)
    {
        // Print source callsign
        printf("\n%s", pAddresses[1].callsign);
        if (pAddresses[1].ssid)
            printf("-%ud", (unsigned int)pAddresses[1].ssid);
        printf(">");

        // Destination callsign
        printf("%s", pAddresses[0].callsign);
        if (pAddresses[0].ssid)
            printf("-%ud", (unsigned int)pAddresses[0].ssid);
        for (i = 2; i < numAddresses; i++)
        {
            printf(",%s", pAddresses[i].callsign);
            if (pAddresses[i].ssid)
                printf("-%ud", (unsigned int)pAddresses[i].ssid);
        }
        printf(":");
    }
}

void AX25::queueFlag()
{
    static const size_t maxPacket = sizeof(m_packet) * 8;
    uint8_t             flag = 0x7e;

    for (int i = 0 ; i < 8 ; i++, m_packetSize++)
    {
        if (m_packetSize >= maxPacket)
        {
            m_frameOverflow = true;
            return;
        }
        if (flag & 1)
            m_packet[m_packetSize >> 3] |= (1 << (m_packetSize & 7));
        else
            m_packet[m_packetSize >> 3] &= ~(1 << (m_packetSize & 7));
        flag >>= 1;
    }
}

void AX25::queueRawByte(uint8_t byte)
{
    static const size_t maxPacket = sizeof(m_packet) * 8;

    for (int i = 0 ; i < 8 ; i++)
    {
        uint8_t bit = byte & 1;
        byte >>= 1;
        updateCRC(bit);
        if (bit)
        {
            // Next bit is a '1'.
            if (m_packetSize >= maxPacket)
            {
                m_frameOverflow = true;
                return;
            }
            m_packet[m_packetSize >> 3] |= (1 << (m_packetSize & 7));
            m_packetSize++;
            if (++m_onesInARow < 5)
                continue;
        }

        // Next bit is a '0' or a zero padding after 5 ones in a row.
        if (m_packetSize >= maxPacket)
        {
            m_frameOverflow = true;
            return;
        }
        m_packet[m_packetSize >> 3] &= ~(1 << (m_packetSize & 7));
        m_packetSize++;
        m_onesInARow = 0;
    }
}

void AX25::updateCRC(uint8_t bit)
{
    m_crc ^= bit;
    if (m_crc & 1)
    {
        // X-modem CRC polynomial.
        m_crc = (m_crc >> 1) ^ 0x8408;
    }
    else
    {
        m_crc = m_crc >> 1;
    }
}

void AX25::queueByte(unsigned char byte)
{
    // Wrap around queueRawByte, but prints debug info.
    queueRawByte(byte);
    if (DEBUG_AX25)
        printf("%c", byte);
}

void AX25::queueString(const char* pString)
{
    while (*pString)
        queueByte(*pString++);
}

void AX25::queueFooter()
{
    // Save the crc so that it can be treated atomically.
    uint16_t finalCRC = ~m_crc;

    // Send the CRC
    queueRawByte(finalCRC & 0xff);
    finalCRC >>= 8;
    queueRawByte(finalCRC & 0xff);

    // Signal the end of frame.
    queueFlag();
    if (DEBUG_AX25)
        printf("\n");
}

void AX25::sendFrame()
{
  // Key the transmitter and send the frame.
  m_afsk.sendData(m_packet, m_packetSize);
}
