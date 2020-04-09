#pragma once
//------------------------------------------------------------------------------
/**
    @class Net::Socket
    
    Platform independent wrapper class for the Sockets API.
    
    (C) 2006 Radon Labs GmbH
    (C) 2013-2020 Individual contributors, see AUTHORS file
*/
#if (__WIN32__)
#include "net/win360/win360socket.h"
namespace Net
{
class Socket : public Win360::Win360Socket
{
    __DeclareClass(Socket);
};
}
#elif __linux__
#include "net/posix/posixsocket.h"
namespace Net
{
class Socket : public Posix::PosixSocket
{
    __DeclareClass(Socket);
};
}
#else
#error "Socket class not implemented on this platform"
#endif
//------------------------------------------------------------------------------
