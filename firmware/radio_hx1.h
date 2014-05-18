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

#ifndef __RADIO_HX1_H__
#define __RADIO_HX1_H__

#include <mbed.h>
#include "radio.h"
#include "radioout.h"

class RadioHx1 : public IRadio
{
public:
    RadioHx1(IRadioOut* pRadioOut, PinName enablePin) : m_enable(enablePin)
    {
        m_pRadioOut = pRadioOut;
        m_enable = 0;
    }

    // IRadio methods.
    virtual void enable();
    virtual void disable();
    virtual void set(uint16_t outputValue);

protected:
    DigitalOut m_enable;
    IRadioOut* m_pRadioOut;
};

#endif // __RADIO_HX1_H__
