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
#include "libraries/galactic_unicorn/galactic_unicorn.hpp"
#include "lwip/dns.h"
#include "lwip/udp.h"

/* Constants. */

#define UC_CONFIG_FILENAME    "config.txt"
#define UC_SSID_MAXLEN        32
#define UC_PASSWORD_MAXLEN    64
#define UC_NTPSERVER_MAXLEN   64

#define UC_CONFIG_CHECK_MS    5000
#define UC_RENDER_MS          250
#define UC_INPUT_DELAY_MS     250
#define UC_DIMMER_MS          5000
#define UC_NTP_CHECK_MS       60000
#define UC_NTP_REFRESH_MS     43200000L
#define UC_NTP_EPOCH_OFFSET   2208988800L
#define UC_NTP_PORT           123
#define UC_NTP_PACKAGE_LEN    48

#define UC_TZ_OFFSET_MAX_MN   840
#define UC_TZ_OFFSET_MIN_MN   -720

#define UC_HUE_MIDDAY         1.1f
#define UC_HUE_MIDNIGHT       0.8f
#define UC_SAT_MIDDAY         1.0f
#define UC_SAT_MIDNIGHT       1.0f
#define UC_VAL_MIDDAY         0.8f
#define UC_VAL_MIDNIGHT       0.3f
#define UC_HUE_OFFSET         -0.12f


typedef enum
{
  UC_DISPLAY_TIME,
  UC_DISPLAY_DATE, UC_DISPLAY_TIMEZONE
} uc_display_mode_t;


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
bool      config_write( const uc_config_t * );
bool      config_changed( uint32_t );

void      display_init( pimoroni::GalacticUnicorn *, pimoroni::PicoGraphics * );
void      display_render( void );
void      display_update_brightness( void );
void      display_dimmer( void );
void      display_brighter( void );
void      display_timezone( void );

void      time_init( void );
bool      time_check_sync( const uc_config_t * );
void      time_set_timezone( const char * );
void      time_set_utc_offset( uc_config_t *, int16_t );
int16_t   time_get_utc_offset( void );


/* End of file uniclock.h */
