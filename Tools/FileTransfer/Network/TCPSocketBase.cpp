/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include <Debug/DVAssert.h>

#include "IOLoop.h"
#include "Endpoint.h"
#include "TCPSocketBase.h"

namespace DAVA
{

TCPSocketBase::TCPSocketBase(IOLoop* ioLoop) : loop(ioLoop), handle()
{
    DVASSERT(ioLoop != NULL);
    uv_tcp_init(loop->Handle(), &handle);
}

bool TCPSocketBase::IsClosed() const
{
    return uv_is_closing(HandleAsHandle()) ? true : false;
}

std::size_t TCPSocketBase::WriteQueueSize() const
{
    return handle.write_queue_size;
}

int32 TCPSocketBase::Bind(const Endpoint& endpoint)
{
    return uv_tcp_bind(Handle(), endpoint.CastToSockaddr(), 0);
}

int32 TCPSocketBase::Bind(const char8* ipaddr, int16 port)
{
    DVASSERT(ipaddr != NULL);

    Endpoint endpoint;
    int32 result = uv_ip4_addr(ipaddr, port, endpoint.CastToSockaddrIn());
    if(0 == result)
    {
        result = Bind(endpoint);
    }
    return result;
}

int32 TCPSocketBase::Bind(int16 port)
{
    return Bind(Endpoint(port));
}

void TCPSocketBase::InternalClose(uv_close_cb callback)
{
    if(!IsClosed())
    {
        uv_close(HandleAsHandle(), callback);
    }
}

void TCPSocketBase::CleanUpBeforeNextUse()
{
    uv_tcp_init(loop->Handle(), &handle);
}

}	// namespace DAVA
