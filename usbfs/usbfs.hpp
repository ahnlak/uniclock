/*
 * usbfs/usbfs.hpp - part of UniClock, a Clock for the Galactic Unicorn.
 *
 * UniClock is an enhance clock / calendar display for the beautiful Galactic
 * Unicorn.
 *
 * usbfs is the subsystem that handles presenting a filesystem to the host
 * over USB; in this instance used for managing configuration files.
 *
 * This header file brings together all the USB storage related stuff; with
 * any luck this will be fairly easily transferable to other projects.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * Released under the MIT License; see LICENSE for details.
 */

#pragma once

#include "ff.h"

/* Constants. */

#define USB_VENDOR_ID       0xCafe
#define USB_VENDOR_STR      "GalacticUnicorn"
#define USB_PRODUCT_STR     "Device"

#define UFS_LABEL           "GUnicorn"


/* Function prototypes. */

void      storage_get_size( uint16_t &, uint32_t & );
int32_t   storage_read( uint32_t, uint32_t, void *, uint32_t );
int32_t   storage_write( uint32_t, uint32_t, const uint8_t *, uint32_t );

void      ufs_init( void );
FRESULT   ufs_mount( void );
FRESULT   ufs_unmount( void );

void      usb_init( void );
void      usb_update( void );
void      usb_debug( const char *, ... );
void      usb_fs_changed( void );


/* End of file usbfs/usbfs.hpp */
