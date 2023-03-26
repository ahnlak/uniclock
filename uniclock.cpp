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
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "libraries/galactic_unicorn/galactic_unicorn.hpp"


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
  absolute_time_t             l_config_check = nil_time;
  absolute_time_t             l_dimmer_check = nil_time;
  absolute_time_t             l_input_delay = nil_time;
  absolute_time_t             l_ntp_check = nil_time;
  absolute_time_t             l_next_render = nil_time;
  pimoroni::PicoGraphics     *l_graphics;
  pimoroni::GalacticUnicorn  *l_unicorn;
  int16_t                     l_new_offset;


  /* Initial setup stuff - first get Unicorn and Graphics objects. */
  l_unicorn = new pimoroni::GalacticUnicorn();
  l_graphics = new pimoroni::PicoGraphics_PenRGB565( 
    pimoroni::GalacticUnicorn::WIDTH,
    pimoroni::GalacticUnicorn::HEIGHT,
    nullptr
  );

  /* And initialise all the subsystems. */
  stdio_init_all();
  ufs_init();
  usb_init();
  time_init();
  display_init( l_unicorn, l_graphics );
  l_unicorn->init();

  /* Fetch the current configuration. */
  m_config_stamp = config_read( &m_config );
  time_set_utc_offset( nullptr, m_config.utc_offset_minutes );

  /* Now enter the main control loop; we normally never leave this. */
  while( true )
  {
    /* Handle any USB-facing work. */
    usb_update();

    /* Other things we do less busily; configuration file changes. */
    if ( time_reached( l_config_check ) )
    {
      /* Check to see if the configuration file has been updated. */
      if ( config_changed( m_config_stamp ) )
      {
        /* Then re-read it. */
        m_config_stamp = config_read( &m_config );

        /* And apply any immediate changes. */
        time_set_utc_offset( nullptr, m_config.utc_offset_minutes );
      }

      /* Schedule the next check for a minutes time. */
      l_config_check = make_timeout_time_ms( UC_CONFIG_CHECK_MS );
    }

    /* Adjust the brightness to reflect the ambient light levels. */
    if ( time_reached( l_dimmer_check ) )
    {
      /* Fairly simple operation. */
      display_update_brightness();

      /* Schedule the next check for a minutes time. */
      l_dimmer_check = make_timeout_time_ms( UC_DIMMER_MS );
    }

    /* Update our RTC via NTP, on occasions. */
    if ( time_reached( l_ntp_check ) )
    {
      /* Ask for a time sync; we may need to call this repeatedly, to allow */
      /* for the wifi to become available.                                  */
      if ( time_check_sync( &m_config ) )
      {
        /* Schedule the next check. */
        l_ntp_check = make_timeout_time_ms( UC_NTP_CHECK_MS );
      }
    }

    /* Process any user input. */
    if ( time_reached( l_input_delay ) )
    {
      /* First up, brightness controls, done on the LUX buttons. */
      if ( l_unicorn->is_pressed( pimoroni::GalacticUnicorn::SWITCH_BRIGHTNESS_UP ) )
      {
        display_brighter();
      }
      if ( l_unicorn->is_pressed( pimoroni::GalacticUnicorn::SWITCH_BRIGHTNESS_DOWN ) )
      {
        display_dimmer();
      }

      /* Adjust the timezone using the volume buttons, like clock.py */
      if ( l_unicorn->is_pressed( pimoroni::GalacticUnicorn::SWITCH_VOLUME_UP ) )
      {
        /* With this basic adjustment, offsets are locked to hour-long steps. */
        l_new_offset = ( ( time_get_utc_offset() / 60 ) + 1 ) * 60;
        time_set_utc_offset( &m_config, l_new_offset );
        display_timezone();
      }
      if ( l_unicorn->is_pressed( pimoroni::GalacticUnicorn::SWITCH_VOLUME_DOWN ) )
      {
        /* With this basic adjustment, offsets are locked to hour-long steps. */
        l_new_offset = ( ( time_get_utc_offset() / 60 ) - 1 ) * 60;
        time_set_utc_offset( &m_config, l_new_offset );
        display_timezone();
      }

      /* Other displays; the 'D' button will briefly show you the date. */
      if ( l_unicorn->is_pressed( pimoroni::GalacticUnicorn::SWITCH_D ) )
      {
        display_date();
      }

      /* Wait a little while until we check again. */
      l_input_delay = make_timeout_time_ms( UC_INPUT_DELAY_MS );
    }

    /* Rendering, which we do fairly leisurely. */
    if ( time_reached( l_next_render ) )
    {
      /* Draw the display. */
      display_render( &m_config );

      /* Push the display out to the unicorn. */
      l_unicorn->update( l_graphics );

      /* And schedule the next render. */
      l_next_render = make_timeout_time_ms( UC_RENDER_MS );
    }
  }

  /* We would usually never expect to reach an end. */
  return 0;
}

/* End of file uniclock.cpp */
