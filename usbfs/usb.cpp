/*
 * usbfs/usb.cpp - part of UniClock, a Clock for the Galactic Unicorn.
 *
 * UniClock is an enhance clock / calendar display for the beautiful Galactic
 * Unicorn.
 *
 * This provides all the USB handling routines; it's largely 'borrowed' from
 * DaftFreak's awesome work on the PicoSystem, but simplified for our needs.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * Released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Local headers. */

#include "tusb.h"
#include "usbfs.hpp"


/* Module variables. */

static bool     m_mounted = true;


/* Functions.*/

/*
 * tinyusb callbacks.
 */

void tud_mount_cb( void )
{  
  m_mounted = true;
}

void tud_msc_inquiry_cb( uint8_t p_lun, uint8_t p_vendor_id[8], 
                         uint8_t p_product_id[16], uint8_t p_product_rev[4] )
{
  const char *l_vid = USB_VENDOR_STR;
  const char *l_pid = USB_PRODUCT_STR " Storage";
  const char *l_rev = "1.0";

  memcpy( p_vendor_id  , l_vid, strlen(l_vid) );
  memcpy( p_product_id , l_pid, strlen(l_pid) );
  memcpy( p_product_rev, l_rev, strlen(l_rev) );
}

bool tud_msc_test_unit_ready_cb( uint8_t p_lun )
{
  if( !m_mounted )
  {
    tud_msc_set_sense( p_lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00 );
    return false;
  }

  return true;
}

void tud_msc_capacity_cb( uint8_t p_lun, 
                          uint32_t *p_block_count, uint16_t *p_block_size )
{
  storage_get_size( *p_block_size, *p_block_count );
}

bool tud_msc_start_stop_cb( uint8_t p_lun, uint8_t p_power_condition,
                            bool p_start, bool p_load_eject )
{
  if( p_load_eject )
  {
    if ( !p_start )
    {
      m_mounted = false;
    }
  }

  return true;
}

int32_t tud_msc_read10_cb( uint8_t p_lun, uint32_t p_lba,
                           uint32_t p_offset, void *p_buffer, uint32_t p_bufsize )
{
  return storage_read( p_lba, p_offset, p_buffer, p_bufsize );
}

int32_t tud_msc_write10_cb( uint8_t p_lun, uint32_t p_lba,
                            uint32_t p_offset, uint8_t *p_buffer, uint32_t p_bufsize )
{
  return storage_write( p_lba, p_offset, p_buffer, p_bufsize );
}

int32_t tud_msc_scsi_cb( uint8_t p_lun, uint8_t const p_scsi_cmd[16],
                         void *p_buffer, uint16_t p_bufsize )
{
  int32_t  l_retval;

  switch ( p_scsi_cmd[0] )
  {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
      // Host is about to read/write etc ... better not to disconnect disk
      l_retval = 0;
      break;

    default:
      // Set Sense = Invalid Command Operation
      tud_msc_set_sense( p_lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00 );

      // negative means error -> tinyusb could stall and/or response with failed status
      l_retval = -1;
      break;
  }

  return l_retval;
}

bool tud_msc_is_writable_cb( uint8_t p_lun )
{
  return true;
}


/*
 * Higher level USB functions; these are the ones exposed to our program.
 */

/*
 * init - called to initialise the tinyusb library.
 */

void usb_init( void )
{
  /* Initialise the tinyusb library. */
  tusb_init();

  /* All done. */
  return;
}

/*
 * update - called regularly to process any outstanding USB work.
 */

void usb_update( void )
{
  /* Ask tinyusb to run any device tasks. */
  tud_task();

  /* All done. */
  return;
}


/*
 * debug - sends a debug message over CDC; we treat this channel as write-only.
 */

void usb_debug( const char *p_message, ... )
{
  va_list   l_args;
  char      l_buffer[64];
  int       l_msglen;
  uint32_t  l_sentbytes;

  /* Assemble the debug message. */
  va_start( l_args, p_message );
  l_msglen = vsnprintf( l_buffer, 60, p_message, l_args );
  strcat( l_buffer, "\r\n" );
  l_msglen += 2;
  va_end( l_args );

  /* And send it to USB. */
  l_sentbytes = tud_cdc_write( l_buffer, l_msglen );
  while ( l_sentbytes < l_msglen )
  {
    tud_task();
    if ( !tud_ready() )
    {
      break;
    }
    l_sentbytes += tud_cdc_write( l_buffer + l_sentbytes, l_msglen - l_sentbytes );
  }

  /* All done. */
  return;
}

/* End of file usb.cpp */
