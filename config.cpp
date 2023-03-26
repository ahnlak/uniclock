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
  usb_debug( "Reading configuration file %s", UC_CONFIG_FILENAME );
  ufs_mount();
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
        strncpy( p_config->wifi_ssid, l_buffer+6, UC_SSID_MAXLEN );
        p_config->wifi_ssid[UC_SSID_MAXLEN] = '\0';
        usb_debug( "Setting SSID to %s", p_config->wifi_ssid );
      }
      if ( strncmp( l_buffer, "PASSWORD: ", 10 ) == 0 )
      {
        strncpy( p_config->wifi_password, l_buffer+10, UC_PASSWORD_MAXLEN );
        p_config->wifi_password[UC_PASSWORD_MAXLEN] = '\0';
        usb_debug( "Setting PASSWORD to %s", p_config->wifi_password );
      }
      if ( strncmp( l_buffer, "NTP_SERVER: ", 12 ) == 0 )
      {
        strncpy( p_config->ntp_server, l_buffer+12, UC_NTPSERVER_MAXLEN );
        p_config->wifi_password[UC_NTPSERVER_MAXLEN] = '\0';
        usb_debug( "Setting NTP_SERVER to %s", p_config->ntp_server );
      }
      if ( strncmp( l_buffer, "UTC_OFFSET: ", 12 ) == 0 )
      {
        p_config->utc_offset_minutes = atoi( l_buffer+12 );
        usb_debug( "Setting UTC_OFFSET to %d", p_config->utc_offset_minutes );
      }
    }

    /* All done. */
    f_close( &l_fptr );
  }
  else
  {
    /* Fill in some defaults for the configuration. */
    strcpy( p_config->wifi_ssid, "unknown" );
    usb_debug( "Defaulting SSID to %s", p_config->wifi_ssid );
    strcpy( p_config->wifi_password, "unknown" );
    usb_debug( "Defaulting PASSWORD to %s", p_config->wifi_password );
    strcpy( p_config->ntp_server, "pool.ntp.org" );
    usb_debug( "Defaulting NTP_SERVER to %s", p_config->ntp_server );
    p_config->utc_offset_minutes = 0;
    usb_debug( "Defaulting UTC_OFFSET to %d", p_config->utc_offset_minutes );

    /* And write them into the file. */
    if ( !config_write( p_config ) )
    {
      ufs_unmount();
      return 0;
    }
  }

  /* Lastly, stat the file and return the timestamp on it. */
  l_result = f_stat( UC_CONFIG_FILENAME, &l_fileinfo );
  if ( l_result != FR_OK )
  {
    ufs_unmount();
    return 0;
  }

  /* Merge fdate and ftime (both 2 bytes long) into a single DWORD. */
  l_filestamp = ( l_fileinfo.fdate << 16 ) | l_fileinfo.ftime;
  ufs_unmount();
  return l_filestamp;
}


/*
 * write - saves the provided configuration, overwriting any config that is
 *         currently there.
 */

bool config_write( const uc_config_t *p_config )
{
  FIL       l_fptr;
  FRESULT   l_result;
  char      l_buffer[128];

  /* So, try to open it for writing, creating as required. */
  ufs_mount();
  l_result = f_open( &l_fptr, UC_CONFIG_FILENAME, FA_CREATE_ALWAYS | FA_WRITE );
  if ( l_result != FR_OK )
  {
    ufs_unmount();
    return false;
  }

  /* And write our values into the file. */
  snprintf( l_buffer, 127, "SSID: %s\n", p_config->wifi_ssid );
  f_puts( l_buffer, &l_fptr );
  snprintf( l_buffer, 127, "PASSWORD: %s\n", p_config->wifi_password );
  f_puts( l_buffer, &l_fptr );
  snprintf( l_buffer, 127, "NTP_SERVER: %s\n", p_config->ntp_server );
  f_puts( l_buffer, &l_fptr );
  snprintf( l_buffer, 127, "UTC_OFFSET: %d\n", p_config->utc_offset_minutes );
  f_puts( l_buffer, &l_fptr );

  /* Close it up. */
  f_close( &l_fptr );
  ufs_unmount();
  usb_fs_changed();

  /* And we're done! */
  return true;
}


/*
 * changed - quick test to compare the provided timestamp with the current one,
 *           to determine if the configuration file has been modified.
 */

bool config_changed( uint32_t p_timestamp )
{
  FILINFO   l_fileinfo;
  FRESULT   l_result;
  uint32_t  l_filestamp;

  /* Stat the configuration file. */
  ufs_mount();
  l_result = f_stat( UC_CONFIG_FILENAME, &l_fileinfo );
  ufs_unmount();
  if ( l_result != FR_OK )
  {
    return true;
  }

  /* Merge fdate and ftime (both 2 bytes long) into a single DWORD. */
  l_filestamp = ( l_fileinfo.fdate << 16 ) | l_fileinfo.ftime;
  if ( p_timestamp == l_filestamp )
  {
    /* The timestamps match, so the file hasn't changed. */
    return false;
  }
  return true;
}


/* End of file config.cpp */
