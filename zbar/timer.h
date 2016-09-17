/*------------------------------------------------------------------------
 *  Copyright 2007-2010 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the ZBar Bar Code Reader.
 *
 *  The ZBar Bar Code Reader is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The ZBar Bar Code Reader is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the ZBar Bar Code Reader; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zbar
 *------------------------------------------------------------------------*/
#ifndef _ZBAR_TIMER_H_
#define _ZBAR_TIMER_H_

#include <time.h>

#if defined(_WIN32)

# include <windows.h>

typedef DWORD zbar_timer_t;

static inline int _zbar_timer_now ()
{
    return(timeGetTime());
}

static inline zbar_timer_t *_zbar_timer_init (zbar_timer_t *timer,
                                              int delay)
{
    if(delay < 0)
        return(NULL);

    *timer = timeGetTime() + delay;
    return(timer);
}

static inline int _zbar_timer_check (zbar_timer_t *timer)
{
    int delay;
    if(!timer)
        return(INFINITE);

    delay = *timer - timeGetTime();
    return((delay >= 0) ? delay : 0);
}

#else

#include <sys/time.h>

typedef struct timeval zbar_timer_t;

static inline int _zbar_timer_now ()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return(now.tv_sec * 1000 + now.tv_usec / 1000);
}

static inline zbar_timer_t *_zbar_timer_init (zbar_timer_t *timer,
                                              int delay)
{
    if(delay < 0)
        return(NULL);

    gettimeofday(timer, NULL);
    timer->tv_usec += (delay % 1000) * 1000;
    timer->tv_sec += (delay / 1000) + (timer->tv_usec / 1000000);
    timer->tv_usec %= 1000000;
    return(timer);
}

static inline int _zbar_timer_check (zbar_timer_t *timer)
{
    struct timeval now;
    if(!timer)
        return(-1);

    gettimeofday(&now, NULL);
    return((timer->tv_sec - now.tv_sec) * 1000 +
           (timer->tv_usec - now.tv_usec) / 1000);
}
#endif
#endif
