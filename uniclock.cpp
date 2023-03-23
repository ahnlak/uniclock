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


/* Functions.*/

/*
 * main - entry point; in this embedded world, we don't have the traditional
 *        arguments (for the simple reason that the concept of command line
 *        options are nonsensical!)
 */

int main()
{
  /* Initial setup stuff. */
  stdio_init_all();
  ufs_init();
  usb_init();

  /* Now enter the main control loop; we normally never leave this. */
  while( true )
  {
    /* Need to keep the USB alive... */
    usb_update();
  }

  /* We would usually never expect to reach an end. */
  return 0;
}

/* End of file uniclock.cpp */
