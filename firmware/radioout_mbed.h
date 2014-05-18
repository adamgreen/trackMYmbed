/* trackuino copyright (C) 2014         Adam Green (https://github.com/adamgreen)
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

#ifndef __RADIO_OUT_MBED_H__
#define __RADIO_OUT_MBED_H__

#include <mbed.h>
#include "radioout.h"

class RadioOutMbed : public IRadioOut
{
public:
    RadioOutMbed(PinName analogPin) : m_output(analogPin)
    {
        m_output = 0.5f;
    }

    // IRadioOut methods.
    virtual void set(uint16_t outputValue);
protected:
    AnalogOut   m_output;
};

#endif // __RADIO_OUT_MBED_H__
