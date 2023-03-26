/*
 * usbfs/ufs.cpp - part of UniClock, a Clock for the Galactic Unicorn.
 *
 * UniClock is an enhance clock / calendar display for the beautiful Galactic
 * Unicorn.
 *
 * usbfs is the subsystem that handles presenting a filesystem to the host
 * over USB; in this instance used for managing configuration files.
 *
 * ufs is the actual filesystem management, which sits on top of the USB and
 * storage components.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * Released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"

/* Local headers. */

#include "ff.h"
#include "diskio.h"
#include "usbfs.hpp"


/* Module variables. */

static FATFS m_fatfs;


/* Functions.*/

/*
 * init - initialises the filesystem; if it's not already, we'll format it.
 */

void ufs_init( void )
{
  FRESULT   l_result;
  MKFS_PARM l_options;

  /* Attempt to mount it. */
  l_result = ufs_mount();

  /* If there was no filesystem, make one. */
  if ( l_result == FR_NO_FILESYSTEM )
  {
    /* Set up the options, and format. */
    l_options.fmt = FM_ANY | FM_SFD;
    l_result = f_mkfs( "", &l_options, m_fatfs.win, FF_MAX_SS );
    if ( l_result != FR_OK )
    {
      return;
    }

    /* And re-mount. */
    ufs_mount();
  }

  /* Set the label on the volume to something sensible. */
  f_setlabel( UFS_LABEL );

  /* But don't leave it mounted. */
  ufs_unmount();

  /* All done. */
  return;
}


/*
 * mount - mount the filesystem.
 */

FRESULT ufs_mount( void )
{
  return f_mount( &m_fatfs, "", 1 );
}


/*
 * unmount - unmount the filesystem.
 */

FRESULT ufs_unmount( void )
{
  return f_mount( 0, "", 0 );
}


/*
 * FatFS glue functions; used to link the filesystem to the storage. This is
 * largely poached (along with a lot of this USB stuff) from @DaftFreak's 
 * amazing work on https://github.com/32blit/32blit-sdk/
 */

DSTATUS disk_initialize(BYTE pdrv) {
  return RES_OK;
}

DSTATUS disk_status(BYTE pdrv) {
  return RES_OK;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
  static_assert(FF_MIN_SS == FF_MAX_SS);
  return storage_read(sector, 0, buff, FF_MIN_SS * count) == int32_t(FF_MIN_SS * count) ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  return storage_write(sector, 0, buff, FF_MIN_SS * count) == int32_t(FF_MIN_SS * count) ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
  uint16_t block_size;
  uint32_t num_blocks;

  switch(cmd) {
    case CTRL_SYNC:
      return RES_OK;

    case GET_SECTOR_COUNT:
      storage_get_size(block_size, num_blocks);
      *(LBA_t *)buff = num_blocks;
      return RES_OK;

    case GET_BLOCK_SIZE:
      *(DWORD *)buff = 1;
      return RES_OK;
  }

  return RES_PARERR;
}

/* End of file ufs.cpp */
