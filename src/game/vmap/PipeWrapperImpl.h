/*
 * Copyright (C) 2017 Hellfire <https://hellfire-core.github.io/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _PIPEWRAPPERIMPL_H
#define _PIPEWRAPPERIMPL_H

#include "ByteBuffer.h"

#include <ace/SPIPE_Acceptor.h>
#include <ace/SPIPE_Connector.h>
#include <ace/SPIPE_Addr.h>

#define ERROR_CONNECT_NO_PIPE   2

#define ERROR_EOF_ON_PIPE       109
#define ERROR_MORE_DATA_IN_PIPE 234

namespace VMAP
{

    template<class STREAM>
    _PipeWrapper<STREAM>::~_PipeWrapper()
    {
        if(m_stream && m_connected)
            Close();

        if(m_stream)
            delete m_stream;
    }

    template<class STREAM>
    void _SynchronizedSendPipeWrapper<STREAM>::Connect(const char* name, int32 *id)
    {
        Guard g(m_lock);
        if(!g.locked())
            sLog.outLog(LOG_DEFAULT, "ERROR: Connect: failed to aquire lock");

        _SendPipeWrapper<STREAM>::Connect(name, id);
    }

    template<class STREAM>
    void _SynchronizedRecvPipeWrapper<STREAM>::Accept(const char* name, int32 *id)
    {
        Guard g(m_lock);
        if(!g.locked())
            sLog.outLog(LOG_DEFAULT, "ERROR: Accept: failed to aquire log");

        _RecvPipeWrapper<STREAM>::Accept(name, id);
    }

    template<class STREAM>
    ByteBuffer _SynchronizedRecvPipeWrapper<STREAM>::RecvPacket()
    {
        Guard g(m_lock);
        if(!g.locked())
        {
            ByteBuffer packet;
            sLog.outLog(LOG_DEFAULT, "ERROR: RecvPacket: failed to aquire lock");
            _RecvPipeWrapper<STREAM>::m_eof = true;
            return packet;
        }
        return _RecvPipeWrapper<STREAM>::RecvPacket();
    }

    template<class STREAM>
    bool _RecvPipeWrapper<STREAM>::recv(ByteBuffer &packet, uint32 size)
    {
        int n;
        while(true)
        {
            n = _PipeWrapper<STREAM>::m_stream->recv_n(m_buffer, size);
            if (n < 0)
            {
                int code = ACE_OS::last_error();
                if(code == ERROR_EOF_ON_PIPE)
                {
                    m_eof = true;
                    return false;
                }
                else if(code == ERROR_MORE_DATA_IN_PIPE)
                {
                    // ignore error
                }
                else
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: recv: failed to recv data from stream because of error %d", code);
                    m_eof = true;
                    return false;
                }
            }
            else if(n == 0)
            {
                ACE_Thread::yield();
                continue;
            }
            break;
        }

        for(uint32 i = 0; i < size; i++)
            packet << m_buffer[i];

        return true;
    }

    template<class STREAM>
    ByteBuffer _RecvPipeWrapper<STREAM>::RecvPacket()
    {
        ByteBuffer packet;
        uint8 size;

        if(!recv(packet, 1))
            return packet;

        size = m_buffer[0];

        if (size > 1)
            if(!recv(packet, size - 1))
                return packet;
        return packet;
    }

    template<class STREAM>
    void _SynchronizedSendPipeWrapper<STREAM>::SendPacket(ByteBuffer &packet)
    {
        Guard g(m_lock);
        if(!g.locked())
            sLog.outLog(LOG_DEFAULT, "ERROR: SendPacket: failed to aquire lock, unintended bahaviour possible");

        return _SendPipeWrapper<STREAM>::SendPacket(packet);
    }

    template<class STREAM>
    void _SendPipeWrapper<STREAM>::SendPacket(ByteBuffer &packet)
    {
        uint32 len = packet.size();
        uint8 *buf = new uint8[len];
        packet.read(buf, len);
        packet.rpos(0);

        uint32 offset = 0;
        while(offset < len)
            offset += _PipeWrapper<STREAM>::m_stream->send(buf + offset, len - offset);

        delete [] buf;
    }

    template<class STREAM>
    void _PipeWrapper<STREAM>::Close()
    {
        m_stream->remove();
        delete m_stream;
        m_stream = 0;
        m_connected = false;
    }
}


#endif
