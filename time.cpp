/*
 * time.cpp - part of UniClock, a Clock for the Galactic Unicorn.
 *
 * UniClock is an enhance clock / calendar display for the beautiful Galactic
 * Unicorn.
 *
 * All time related things here, initialising and managing the RTC as well as
 * all the processing around NTP requests and applying timezones.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * Released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/rtc.h"


/* Local headers. */

#include "uniclock.h"
#include "usbfs.hpp"


/* Module variables. */

static absolute_time_t    m_next_ntp_check = nil_time;


/* Functions.*/

/*
 * init - sets up the time related things.
 */

void time_init( void )
{
  datetime_t l_time;

  /* Initialise the RTC */
  rtc_init();

  /* It also needs a valid time setting, before it runs. */
  l_time.year = 2023;
  l_time.month = 1;
  l_time.day = 1;
  l_time.dotw = 0;
  l_time.hour = l_time.min = l_time.sec = 0;
  rtc_set_datetime( &l_time );

  /* All done. */
  return;
}


/*
 * check_sync - if we haven't updated from NTP in some time, initiate it.
 *              returns true once we have either successfully updated, or
 *              failed in a fairly firm way.
 */

bool time_check_sync( const uc_config_t *p_config )
{
  /* If we're not due to sync, just say true right away. */
  if ( !time_reached( m_next_ntp_check ) )
  {
    return true;
  }

  /* TESTING - we'll fake an answer here... */
  if ( true )
  {
    static datetime_t l_datetime;

    /* The time will be in seconds since 1900; convert it to a tmstruct. */
    l_datetime.year  = 2023;
    l_datetime.month = 1;
    l_datetime.day   = 1;
    l_datetime.dotw  = 0;
    l_datetime.hour  = 16;
    l_datetime.min   = 14;
    l_datetime.sec   = 12;

    rtc_set_datetime( &l_datetime );

    /* Schedule the next NTP sync for the future... */
    m_next_ntp_check = make_timeout_time_ms( UC_NTP_REFRESH_MS );
    return true;
  }

  /* Not a failure - just means we have more work to do. */
  return false;
}


/* End of file time.cpp */
