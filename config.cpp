/*
 * config.cpp - part of UniClock, a Clock for the Galactic Unicorn.
 *
 * UniClock is an enhance clock / calendar display for the beautiful Galactic
 * Unicorn.
 *
 * These functions handle the reading/writing of our configuration file, to make
 * it easy for the end user to configure their clock.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * Released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Local headers. */

#include "uniclock.h"
#include "usbfs.hpp"


/* Functions.*/

/*
 * read - attempts to read the configuration file; if it doesn't exist, it 
 *        will be created with some reasonable defaults. Returns a time/size
 *        stamp that can be used to identify when the file is changed.
 */

uint32_t  config_read( uc_config_t *p_config )
{
  FIL       l_fptr;
  FILINFO   l_fileinfo;
  FRESULT   l_result;
  char      l_buffer[128];
  uint32_t  l_filestamp;

  /* Try to open up the file. */
  l_result = f_open( &l_fptr, UC_CONFIG_FILENAME, FA_READ );
  if ( l_result == FR_OK )
  {
    /* Should be able to scan one line at a time then. */
    while ( f_gets( l_buffer, 127, &l_fptr ) != nullptr )
    {
      /* Clip off any trailing newline. */
      if ( l_buffer[strlen(l_buffer)-1] == '\n' )
      {
        l_buffer[strlen(l_buffer)-1] = '\0';
      }

      /* Simple match on the start of the line. */
      if ( strncmp( l_buffer, "SSID: ", 6 ) == 0 )
      {
        strncpy( p_config->wifi_ssid, l_buffer, UC_SSID_MAXLEN );
        p_config->wifi_ssid[UC_SSID_MAXLEN] = '\0';
      }
      if ( strncmp( l_buffer, "PASSWORD: ", 10 ) == 0 )
      {
        strncpy( p_config->wifi_password, l_buffer, UC_PASSWORD_MAXLEN );
        p_config->wifi_password[UC_PASSWORD_MAXLEN] = '\0';
      }
    }

    /* All done. */
    f_close( &l_fptr );
  }
  else
  {
    /* Probably means the file doesn't exist - so, err, create it? */
    l_result = f_open( &l_fptr, UC_CONFIG_FILENAME, FA_CREATE_ALWAYS | FA_WRITE );
    if ( l_result != FR_OK )
    {
      return 0;
    }

    /* Fill in some defaults for the configuration. */
    strcpy( p_config->wifi_ssid, "unknown" );
    strcpy( p_config->wifi_password, "unknown" );

    /* And write them into the file. */
    snprintf( l_buffer, 127, "SSID: %s\n", p_config->wifi_ssid );
    f_puts( l_buffer, &l_fptr );
    snprintf( l_buffer, 127, "PASSWORD: %s\n", p_config->wifi_password );
    f_puts( l_buffer, &l_fptr );

    /* Close it up. */
    f_close( &l_fptr );
  }

  /* Lastly, stat the file and return the timestamp on it. */
  l_result = f_stat( UC_CONFIG_FILENAME, &l_fileinfo );
  if ( l_result != FR_OK )
  {
    return 0;
  }

  /* Merge fdate and ftime (both 2 bytes long) into a single DWORD. */
  l_filestamp = ( l_fileinfo.fdate << 16 ) & l_fileinfo.ftime;
  return l_filestamp;
}


/*
 * changed - quick test to compare the provided timestamp with the current one,
 *           to determine if the configuration file has been modified.
 */

bool config_changed( uint32_t p_timestamp )
{
  return false;
}


/* End of file config.cpp */
