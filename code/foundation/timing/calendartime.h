#pragma once
//------------------------------------------------------------------------------
/**
    @class Timing::CalendarTime
    
    Allows to obtain the current point in time as year, month, day, etc...
    down to milliseconds, convert between filetime and CalendarTime, and
    format the time to a human readable string.
    
    (C) 2007 Radon Labs GmbH
    (C) 2013-2020 Individual contributors, see AUTHORS file
*/
#include "core/config.h"
#if __WIN32__
#include "timing/win360/win360calendartime.h"
namespace Timing
{
class CalendarTime : public Win360::Win360CalendarTime
{ };
}
#elif __linux__
#include "timing/posix/posixcalendartime.h"
namespace Timing
{
class CalendarTime : public Posix::PosixCalendarTime
{ };
}
#else
#error "Timing::CalendarTime not implemented on this platform!"
#endif
//------------------------------------------------------------------------------
