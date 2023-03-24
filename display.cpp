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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/rtc.h"

/* Local headers. */

#include "uniclock.h"
#include "usbfs.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "font8_data.hpp"


/* Module variables. */

static pimoroni::PicoGraphics  *m_graphics;
static int                      m_black_pen, m_white_pen;


/* Functions.*/

/*
 * read - attempts to read the configuration file; if it doesn't exist, it 
 *        will be created with some reasonable defaults. Returns a time/size
 *        stamp that can be used to identify when the file is changed.
 */

/*
 * init - sets up the display handling.
 */

void display_init( pimoroni::PicoGraphics *p_graphics )
{
  /* Keep a reference to the graphics. */
  m_graphics = p_graphics;

  /* Create our black and white pens. */
  m_black_pen = m_graphics->create_pen( 0, 0, 0 );
  m_white_pen = m_graphics->create_pen( 255, 255, 255 );

  /* And set the font and other basics. */
  m_graphics->set_font( &font8 );

  /* All done. */
  return;
}


/*
 * render - draws the current display onto the provided graphics context.
 */

void display_render( void )
{
  datetime_t      l_time;
  char            l_buffer[16];
  static bool     l_blink = true;

  /* First, clear the screen. */
  m_graphics->set_pen( m_black_pen );
  m_graphics->clear();

  /* Now, we'll need to know the current time. */
  rtc_get_datetime( &l_time );
  snprintf( l_buffer, 15, "%02d%02d%02d", l_time.hour, l_time.min, l_time.sec );

  /* Render each digit individually, to ensure they're fixed width. */
  m_graphics->set_pen( m_white_pen );
  m_graphics->character( l_buffer[0], pimoroni::Point( 10, 2 ), 1 );
  m_graphics->character( l_buffer[1], pimoroni::Point( 15, 2 ), 1 );

  m_graphics->character( l_buffer[2], pimoroni::Point( 22, 2 ), 1 );
  m_graphics->character( l_buffer[3], pimoroni::Point( 27, 2 ), 1 );

  m_graphics->character( l_buffer[4], pimoroni::Point( 34, 2 ), 1 );
  m_graphics->character( l_buffer[5], pimoroni::Point( 39, 2 ), 1 );

  /* Add blinking separators, to show we're alive. */
  if ( l_blink )
  {
    m_graphics->pixel( pimoroni::Point( 20, 4 ) ); 
    m_graphics->pixel( pimoroni::Point( 20, 6 ) );

    m_graphics->pixel( pimoroni::Point( 32, 4 ) );
    m_graphics->pixel( pimoroni::Point( 32, 6 ) );

  }
  l_blink = !l_blink;

  /* All done. */
  return;
}


/* End of file display.cpp */
