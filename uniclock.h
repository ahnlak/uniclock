/*
 * uniclock.h - part of UniClock, a Clock for the Galactic Unicorn.
 *
 * UniClock is an enhance clock / calendar display for the beautiful Galactic
 * Unicorn.
 *
 * This is the main header file, which provides a bunch of hopefully useful
 * definitions.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * Released under the MIT License; see LICENSE for details.
 */

#pragma once

#include "libraries/pico_graphics/pico_graphics.hpp"
#include "lwip/dns.h"
#include "lwip/udp.h"

/* Constants. */

#define UC_CONFIG_FILENAME    "config.txt"
#define UC_SSID_MAXLEN        32
#define UC_PASSWORD_MAXLEN    64
#define UC_NTPSERVER_MAXLEN   64

#define UC_CONFIG_CHECK_MS    5000
#define UC_RENDER_MS          250
#define UC_NTP_CHECK_MS       60000
#define UC_NTP_REFRESH_MS     60000
//#define UC_NTP_REFRESH_MS     3600000
#define UC_NTP_EPOCH_OFFSET   2208988800L
#define UC_NTP_PORT           123
#define UC_NTP_PACKAGE_LEN    48


/* Structures. */

typedef struct
{
  char    wifi_ssid[UC_SSID_MAXLEN+1];
  char    wifi_password[UC_PASSWORD_MAXLEN+1];
  char    ntp_server[UC_NTPSERVER_MAXLEN+1];
  int16_t utc_offset_minutes;
} uc_config_t;

typedef struct
{
  ip_addr_t       server;
  struct udp_pcb *socket;
  uint32_t        ntptime;
  bool            active_query;
} uc_ntpstate_t;

/* Function prototypes. */

uint32_t  config_read( uc_config_t * );
bool      config_changed( uint32_t );

void      display_init( pimoroni::PicoGraphics * );
void      display_render( void );

void      time_init( void );
bool      time_check_sync( const uc_config_t * );


/* End of file uniclock.h */
