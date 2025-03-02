/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2011-2019 Pierre Ossman for Cendio AB
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stddef.h>

#include <core/time.h>

#if defined(_MSC_VER)
int gettimeofday(struct timeval* tv, struct timezone* tz)
{
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag;

    if (NULL != tv)
    {
        GetSystemTimeAsFileTime(&ft);

        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        /*converting file time to unix epoch*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS;
        tmpres /= 10;  /*convert into microseconds*/
        tv->tv_sec = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
    }

    if (NULL != tz)
    {
        TIME_ZONE_INFORMATION tz_winapi;
        //_tzset(),don't work properly, so we use GetTimeZoneInformation
        int rez = GetTimeZoneInformation(&tz_winapi);
        tz->tz_dsttime = (rez == 2) ? TRUE : FALSE;
        tz->tz_minuteswest = tz_winapi.Bias + ((rez == 2) ? tz_winapi.DaylightBias : 0);
    }

    return 0;
}
#endif // #if defined(_MSC_VER)

namespace core {

  unsigned msBetween(const struct timeval *first,
                     const struct timeval *second)
  {
    unsigned udiff;

    if (isBefore(second, first))
      return 0;

    udiff = (second->tv_sec - first->tv_sec) * 1000000 +
            (second->tv_usec - first->tv_usec);

    return (udiff + 999) / 1000;
  }

  unsigned msSince(const struct timeval *then)
  {
    struct timeval now;

    gettimeofday(&now, nullptr);

    return msBetween(then, &now);
  }

  unsigned msUntil(const struct timeval *then)
  {
    struct timeval now;

    gettimeofday(&now, nullptr);

    return msBetween(&now, then);
  }

  bool isBefore(const struct timeval *first,
                const struct timeval *second)
  {
    if (first->tv_sec < second->tv_sec)
      return true;
    if (first->tv_sec > second->tv_sec)
      return false;
    if (first->tv_usec < second->tv_usec)
      return true;
    return false;
  }

  struct timeval addMillis(struct timeval inTime, int millis)
  {
    int secs = millis / 1000;
    millis = millis % 1000;
    inTime.tv_sec += secs;
    inTime.tv_usec += millis * 1000;
    if (inTime.tv_usec >= 1000000) {
      inTime.tv_sec++;
      inTime.tv_usec -= 1000000;
    }
    return inTime;
  }

}
