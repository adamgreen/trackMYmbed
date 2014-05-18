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
#include "radio_hx1.h"


void RadioHx1::enable()
{
    m_enable = 1;
    // The HX1 takes 5 ms from PTT to full RF, give it 25
    wait_ms(25);
}

void RadioHx1::disable()
{
    m_enable = 0;
}

void RadioHx1::set(uint16_t outputValue)
{
    m_pRadioOut->set(outputValue);
}
