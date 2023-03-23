/*
 * usbfs/storage.cpp - part of UniClock, a Clock for the Galactic Unicorn.
 *
 * UniClock is an enhance clock / calendar display for the beautiful Galactic
 * Unicorn.
 *
 * usbfs is the subsystem that handles presenting a filesystem to the host
 * over USB; in this instance used for managing configuration files.
 *
 * This is the storage component, which uses flash to hold data.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * Released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/flash.h"
#include "hardware/sync.h"


/* Local headers. */

#include "usbfs.hpp"


/* Module variables. */

static const uint32_t m_storage_size = PICO_FLASH_SIZE_BYTES / 4;
static const uint32_t m_storage_offset = PICO_FLASH_SIZE_BYTES - m_storage_size;


/* Functions.*/

/*
 * get_size - provides size information about storage. 
 */

void storage_get_size( uint16_t &p_block_size, uint32_t &p_num_blocks )
{
  /* Very simple look up. */
  p_block_size = FLASH_SECTOR_SIZE;
  p_num_blocks = m_storage_size / FLASH_SECTOR_SIZE;

  /* All done. */
  return;
}


/*
 * read - fetches data from flash.
 */

int32_t storage_read( uint32_t p_sector, uint32_t p_offset, 
                      void *p_buffer, uint32_t p_size_bytes )
{
  /* Very simple copy out of flash then! */
  memcpy( 
    p_buffer, 
    (uint8_t *)XIP_NOCACHE_NOALLOC_BASE + m_storage_offset + p_sector * FLASH_SECTOR_SIZE + p_offset, 
    p_size_bytes
  );

  /* And return how much we read. */
  return p_size_bytes;
}


/*
 * write - stores data into flash, with appropriate guards. 
 */

int32_t storage_write( uint32_t p_sector, uint32_t p_offset,
                       const uint8_t *p_buffer, uint32_t p_size_bytes )
{
  uint32_t l_status;

  /* Don't want to be interrupted. */
  l_status = save_and_disable_interrupts();

  /* Erasing with an offset of 0? Seems odd, but... */
  if ( p_offset == 0 )
  {
    flash_range_erase( m_storage_offset + p_sector * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE );
  }

  /* And just write the data now. */
  flash_range_program( 
    m_storage_offset + p_sector * FLASH_SECTOR_SIZE + p_offset, 
    p_buffer, p_size_bytes
  );

  /* Lastly, restore our interrupts. */
  restore_interrupts( l_status );

  /* Before returning the amount of data written. */
  return p_size_bytes;
}


/* End of file storage.cpp */
