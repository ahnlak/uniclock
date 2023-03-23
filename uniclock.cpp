/*
 * uniclock.cpp - part of UniClock, a Clock for the Galactic Unicorn.
 *
 * UniClock is an enhance clock / calendar display for the beautiful Galactic
 * Unicorn.
 *
 * This is the main entrypoint to the application.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * Released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"

/* Local headers. */

#include "uniclock.h"
#include "usbfs.hpp"


/* Module variables. */

static uc_config_t  m_config;
static uint32_t     m_config_stamp;


/* Functions.*/

/*
 * main - entry point; in this embedded world, we don't have the traditional
 *        arguments (for the simple reason that the concept of command line
 *        options are nonsensical!)
 */

int main()
{
  absolute_time_t   l_config_check = nil_time;
  absolute_time_t   l_next_render = nil_time;


  /* Initial setup stuff. */
  stdio_init_all();
  ufs_init();
  usb_init();

  /* Fetch the current configuration. */
  m_config_stamp = config_read( &m_config );


  /* Now enter the main control loop; we normally never leave this. */
  while( true )
  {
    /* Handle any USB-facing work. */
    usb_update();

    /* Other things we do less busily; configuration file changes. */
    if ( time_reached( l_config_check ) )
    {
      /* Check to see if the configuration file has been updated. */
      usb_debug( "Checking if configuration file has changed..." );

      if ( config_changed( m_config_stamp ) )
      {
        /* Then re-read it. */
        m_config_stamp = config_read( &m_config );

        /* And apply any immediate changes. */

      }

      /* Schedule the next check for a minutes time. */
      l_config_check = make_timeout_time_ms( UC_CONFIG_CHECK_MS );
    }

    /* Rendering, which we do fairly leisurely. */
    if ( time_reached( l_next_render ) )
    {
      /* Draw the display. */

      /* And schedule the next render. */
      l_next_render = make_timeout_time_ms( UC_RENDER_MS );
    }
  }

  /* We would usually never expect to reach an end. */
  return 0;
}

/* End of file uniclock.cpp */
