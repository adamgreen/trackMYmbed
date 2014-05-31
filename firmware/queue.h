/*  Copyright (C) 2014  Adam Green (https://github.com/adamgreen)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <assert.h>
#include <stdint.h>
#include "config.h"
#include "interlock.h"


class Queue
{
public:
    Queue()
    {
        // GPS_QUEUE_SIZE must be multiple of 2
        assert ( (GPS_QUEUE_SIZE & (GPS_QUEUE_SIZE - 1)) == 0 );

        m_elementWrite = 0;
        m_elementRead = 0;
        m_elementsUsed = 0;
        m_elementsMax = 0;
        m_elementsDropped = 0;
    }

    bool enqueue(char c)
    {
        assert ( m_elementsUsed <= GPS_QUEUE_SIZE );
        if (m_elementsUsed >= GPS_QUEUE_SIZE)
        {
            interlockedIncrement(&m_elementsDropped);
            return false;
        }

        uint32_t elementWrite = m_elementWrite;
        m_buffer[elementWrite] = c;
        m_elementWrite = (elementWrite + 1) & (GPS_QUEUE_SIZE - 1);
        uint32_t usedCount = interlockedIncrement(&m_elementsUsed);
        if (DEBUG_QUEUE && usedCount > m_elementsMax)
            m_elementsMax = usedCount;
        return true;
    }
    
    char dequeue()
    {
        // Shouldn't be called on an empty queue.
        assert ( m_elementsUsed <= GPS_QUEUE_SIZE );
        assert ( m_elementsUsed > 0 );
        if (m_elementsUsed == 0)
            return '\0';

        uint32_t elementRead = m_elementRead;
        char c = m_buffer[elementRead];
        m_elementRead = (elementRead + 1) & (GPS_QUEUE_SIZE - 1);
        interlockedDecrement(&m_elementsUsed);
        return c;
    }

    bool hasData()
    {
        return m_elementsUsed != 0;
    }

    uint32_t droppedElementCount()
    {
        uint32_t elementsDropped = m_elementsDropped;
        interlockedSubtract(&m_elementsDropped, elementsDropped);
        return elementsDropped;
    }

protected:
    volatile uint32_t   m_elementWrite;
    volatile uint32_t   m_elementRead;
    volatile uint32_t   m_elementsUsed;
    volatile uint32_t   m_elementsDropped;
    volatile uint32_t   m_elementsMax;
    volatile char       m_buffer[GPS_QUEUE_SIZE];
};

#endif // __QUEUE_H__
