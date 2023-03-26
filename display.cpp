/*
 * display.cpp - part of UniClock, a Clock for the Galactic Unicorn.
 *
 * UniClock is an enhance clock / calendar display for the beautiful Galactic
 * Unicorn.
 *
 * Here is where we do all the work to draw on the display; all the drawing
 * is done on the provided PicoGraphics object - it's the caller's responsibility
 * to hand that off to the Unicorn itself.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * Released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/rtc.h"

/* Local headers. */

#include "uniclock.h"
#include "usbfs.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "libraries/galactic_unicorn/galactic_unicorn.hpp"
#include "clockfont_data.hpp"


/* Module variables. */

static pimoroni::PicoGraphics    *m_graphics;
static pimoroni::GalacticUnicorn *m_unicorn;
static int                        m_black_pen, m_white_pen;
static float                      m_base_brightness;
static uint_fast8_t               m_brightness_display, m_mode_timer;
static uc_display_mode_t          m_display_mode;


/* Local functions. */

/*
 * calc_midday_percent - works out the percentage to midday, which is used
 *                       to configure the colours shown.
 */

static float display_calc_midday_percent( datetime_t *p_datetime )
{
  uint_fast16_t   l_secs_in_day;
  float           l_day_percent, l_midday_percent;

  /* Work out how many seconds through the day we are. */
  l_secs_in_day = ( ( ( p_datetime->hour * 60 ) + p_datetime->min ) * 60 ) + p_datetime->sec;

  /* Calculate what percent of the day that represents. */
  l_day_percent = l_secs_in_day / 86400.0f;

  /* And finally, translate that to how close to midday we are. */
  l_midday_percent = 1.0f - ( ( cos( l_day_percent * 3.14159 * 2 ) + 1 ) / 2 );

  /* All done! */
  return l_midday_percent;
}


/*
 * create_gradient_pen - creates a pen with a suitable gradient colour, based
 *                       on the midday percentage and the column.
 */

static int create_gradient_pen( float p_midday_percent, int p_column )
{
  int           l_center_proximity, l_midpoint;
  float         l_hue, l_sat, l_val;
  uint_fast8_t  l_red, l_green, l_blue;

  /* Stuff I don't really understand from Pimoroni's HSV/RGB conversions! */
  float         i, f;
  uint_fast8_t  p, q, t;

  /* Work out the base HSV values. */
  l_hue = ( ( UC_HUE_MIDDAY - UC_HUE_MIDNIGHT ) * p_midday_percent ) + UC_HUE_MIDNIGHT;
  l_sat = ( ( UC_SAT_MIDDAY - UC_SAT_MIDNIGHT ) * p_midday_percent ) + UC_SAT_MIDNIGHT;
  l_val = ( ( UC_VAL_MIDDAY - UC_VAL_MIDNIGHT ) * p_midday_percent ) + UC_VAL_MIDNIGHT;

  /* The hue then varies based on the column. */
  l_midpoint = pimoroni::GalacticUnicorn::WIDTH / 2;
  l_center_proximity = l_midpoint - abs( p_column - l_midpoint );
  l_hue += ( UC_HUE_OFFSET * l_center_proximity / l_midpoint );

  /* Convert all this into RGB (with ... weird code) */
  i = floor( l_hue * 6.0f );
  f = l_hue * 6.0f - i;
  l_val *= 255.0f;

  p = l_val * ( 1.0f - l_sat );
  q = l_val * ( 1.0f - f * l_sat );
  t = l_val * ( 1.0f - (1.0f - f) * l_sat );

  switch( int( i ) % 6 ) 
  {
    case 0: l_red = l_val;  l_green = t;      l_blue = p;     break;
    case 1: l_red = q;      l_green = l_val;  l_blue = p;     break;
    case 2: l_red = p;      l_green = l_val;  l_blue = t;     break;
    case 3: l_red = p;      l_green = q;      l_blue = l_val; break;
    case 4: l_red = t;      l_green = p;      l_blue = l_val; break;
    case 5: l_red = l_val;  l_green = p;      l_blue = q;     break;    
  }

  /* Ok, end of that horrid bit of copypasta. Just build the pen. */
  return m_graphics->create_pen( l_red, l_green, l_blue );
}


/* Functions.*/

/*
 * read - attempts to read the configuration file; if it doesn't exist, it 
 *        will be created with some reasonable defaults. Returns a time/size
 *        stamp that can be used to identify when the file is changed.
 */

/*
 * init - sets up the display handling.
 */

void display_init( pimoroni::GalacticUnicorn *p_unicorn, pimoroni::PicoGraphics *p_graphics )
{
  /* Keep a reference to the graphics and the board. */
  m_unicorn = p_unicorn;
  m_graphics = p_graphics;

  /* Create our black and white pens. */
  m_black_pen = m_graphics->create_pen( 0, 0, 0 );
  m_white_pen = m_graphics->create_pen( 255, 255, 255 );

  /* And set the font and other basics. */
  m_graphics->set_font( &clockfont );
  m_base_brightness = 0.5f;
  m_brightness_display = m_mode_timer = 0;
  m_display_mode = UC_DISPLAY_TIME;

  /* All done. */
  return;
}


/*
 * render - draws the current display onto the provided graphics context.
 */

void display_render( const uc_config_t *p_config )
{
  datetime_t      l_time;
  char            l_buffer[16];
  static bool     l_blink = true;
  uint_fast8_t    l_index, l_digit_offset;
  uint_fast8_t    l_row, l_column, l_length;
  float           l_midday_percent;

  /* First, clear the screen. */
  m_graphics->set_pen( m_black_pen );
  m_graphics->clear();

  /* Exactly what we draw now depends on our display mode. */
  switch( m_display_mode )
  {
    case  UC_DISPLAY_TIMEZONE:
      /*
       * Display the current timezone; textual if we have one, but falling
       * back to a simple "UTC+n" if not.
       */

      /* First off, grab the timezone to work out what sort of display. */
      if ( false )
      {
        /* Handle potentially long text string timezones... */
      }
      else
      {
        /* Then we just show it as an offset. */
        snprintf( l_buffer, 15, "UTC%+d", time_get_utc_offset() / 60 );

        /* Work out how big it is, to centralise. */
        l_length = m_graphics->measure_text( l_buffer, 1 );

        /* And just simply draw it. */
        m_graphics->set_pen( m_white_pen );
        m_graphics->text( l_buffer, 
                          pimoroni::Point
                          (
                            ( pimoroni::GalacticUnicorn::WIDTH - l_length ) / 2, 2
                          ),
                          l_length, 1
                        );

        /* And keep ticking down the display. */
        m_mode_timer--;
        if ( m_mode_timer == 0 )
        {
          m_display_mode = UC_DISPLAY_TIME;
        }

      }

      break;

    case UC_DISPLAY_DATE:
      /*
       * Display the date; I suppose in the name of being nice to insane nations,
       * we should allow odd formats here.
       */

      /* We'll need the time, obviously. */
      rtc_get_datetime( &l_time );

      /* Format it appropriately. */
      if ( strcmp( p_config->date_format, "mdy" ) == 0 )
      {
        /* Nasty American ordering. Should be illegal. */
        snprintf( l_buffer, 15, "%02d/%02d/%04d", l_time.month, l_time.day, l_time.year );
      }
      else
      {
        /* Default to the most sensible option... */
        snprintf( l_buffer, 15, "%02d/%02d/%04d", l_time.day, l_time.month, l_time.year );
      }

      /* And just simply draw it. */
      l_length = m_graphics->measure_text( l_buffer, 1 );
      m_graphics->set_pen( m_white_pen );
      m_graphics->text( l_buffer, 
                        pimoroni::Point
                        (
                          ( pimoroni::GalacticUnicorn::WIDTH - l_length ) / 2, 2
                        ),
                        l_length, 1
                      );

      /* And keep ticking down the display. */
      m_mode_timer--;
      if ( m_mode_timer == 0 )
      {
        m_display_mode = UC_DISPLAY_TIME;
      }

      break;

    case UC_DISPLAY_TIME:
      /*
       * Display the time, in white, with a coloured background heavily
       * inspired by the original clock.py demo from Pimoroni.
       */

      /* Now, we'll need to know the current time. */
      rtc_get_datetime( &l_time );
      snprintf( l_buffer, 15, "%02d:%02d:%02d", 
                l_time.hour, l_time.min, l_time.sec );

      /* Render each digit individually, to ensure they're fixed width. */
      m_graphics->set_pen( m_white_pen );
      m_graphics->text( l_buffer, pimoroni::Point( 10, 2 ), pimoroni::GalacticUnicorn::WIDTH, 1 );

      /* Add blinking separators, to show we're alive. */
      if ( l_blink )
      {
        m_graphics->set_pen( m_black_pen );
        m_graphics->pixel( pimoroni::Point( 20, 4 ) ); 
        m_graphics->pixel( pimoroni::Point( 20, 6 ) );

        m_graphics->pixel( pimoroni::Point( 32, 4 ) );
        m_graphics->pixel( pimoroni::Point( 32, 6 ) );

      }
      l_blink = !l_blink;

      /*
       * The gradient background changes based on the current time of day,
       * with a nice fade into the centre.
       */
      l_midday_percent = display_calc_midday_percent( &l_time );

      for ( l_column = 0; l_column < pimoroni::GalacticUnicorn::WIDTH; l_column++ )
      {
        /* Set the right colour. */
        m_graphics->set_pen( create_gradient_pen( l_midday_percent, l_column ) );

        /* Always draw the top and bottom bars. */
        m_graphics->pixel( pimoroni::Point( l_column, 0 ) );
        m_graphics->pixel( pimoroni::Point( l_column, pimoroni::GalacticUnicorn::HEIGHT - 1 ) );

        /* At the edges, full height. */
        if ( ( l_column < 8 ) || ( l_column > 44 ) )
        {
          for ( l_row = 1; l_row < pimoroni::GalacticUnicorn::HEIGHT - 1; l_row++ )
          {
            m_graphics->pixel( pimoroni::Point( l_column, l_row ) );
          }
        }

        /* And lastly, round the corners. */
        if ( ( l_column == 8 ) || ( l_column == 44 ) )
        {
          m_graphics->pixel( pimoroni::Point( l_column, 1 ) );
          m_graphics->pixel( pimoroni::Point( l_column, pimoroni::GalacticUnicorn::HEIGHT - 2 ) );          
        }
      }

      break;
  }

  /* 
   * If the brightness adjustment display is required, draw that. We do this at
   * the end to make sure it overlays on whatever else we're showing.
   */
  if ( m_brightness_display > 0 )
  {
    /* Draw a vertical bar, height based on current brightness. */
    m_graphics->set_pen( m_white_pen );
    for ( l_index = 0; l_index < pimoroni::GalacticUnicorn::HEIGHT; l_index++ )
    {
      if ( l_index <= ( m_base_brightness * pimoroni::GalacticUnicorn::HEIGHT ) )
      {
        m_graphics->pixel( pimoroni::Point(
          pimoroni::GalacticUnicorn::WIDTH - 1,
          pimoroni::GalacticUnicorn::HEIGHT - l_index - 1
        ) );
      }
    }

    /* Count down the frames so we eventually stop displaying it. */
    m_brightness_display--;
  }

  /* All done. */
  return;
}


/*
 * update_brightness - called intermittently, to modify the brightness based
 *                     on ambient light. We don't need to do this constantly,
 *                     it's primarily to auto-dim the display at night.
 */

void display_update_brightness( void )
{
  float   l_ambient_adjustment;
  float   l_brightness;

  /* Work out an adjustment factor, based on the current light levels. */
  l_ambient_adjustment = ( m_unicorn->light() + 512.0f ) / 2048.0f;

  /* And apply that to the base brightness. */
  l_brightness = m_base_brightness * l_ambient_adjustment;

  /* Sanity check that it's not too low. */
  if ( l_brightness < 0.1f )
  {
    l_brightness = 0.1f;
  }

  /* Good; now just set it. */
  m_unicorn->set_brightness( l_brightness );

  /* All done. */
  return;
}


/*
 * dimmer - reduces the target brightness, which is adjusted by ambient light.
 */

void display_dimmer( void )
{
  /* Simply reduce the base brightness, unless it's already at the lowest. */
  if ( m_base_brightness > 0.1f )
  {
    m_base_brightness -= 0.1f;
  }

  /* We want to render some visual feedback to the change. */
  m_brightness_display = 5;

  /* And update the brightness immediately. */
  display_update_brightness();

  /* All done. */
  return;
}


/*
 * brighter - increases the target brightness, which is adjusted by ambient light.
 */

void display_brighter( void )
{
  /* Simply increase the base brightness, unless it's already at the highest. */
  if ( m_base_brightness < 1.0f )
  {
    m_base_brightness += 0.1f;
  }

  /* We want to render some visual feedback to the change. */
  m_brightness_display = 5;

  /* And update the brightness immediately. */
  display_update_brightness();

  /* All done. */
  return;  
}


/*
 * timezone - show the currently set timezone, for a period of time.
 */

void display_timezone( void )
{
  /* Just switch display mode, and set the display timer. */
  m_display_mode = UC_DISPLAY_TIMEZONE;
  m_mode_timer = 5;

  /* All done. */
  return;
}

/*
 * date - show the current date, for a period of time.
 */

void display_date( void )
{
  /* Just switch display mode, and set the display timer. */
  m_display_mode = UC_DISPLAY_DATE;
  m_mode_timer = 10;

  /* All done. */
  return;
}


/* End of file display.cpp */
