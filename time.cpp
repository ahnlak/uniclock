/*
 * time.cpp - part of UniClock, a Clock for the Galactic Unicorn.
 *
 * UniClock is an enhance clock / calendar display for the beautiful Galactic
 * Unicorn.
 *
 * All time related things here, initialising and managing the RTC as well as
 * all the processing around NTP requests and applying timezones.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * Released under the MIT License; see LICENSE for details.
 */

/* System headers. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/rtc.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"


/* Local headers. */

#include "uniclock.h"
#include "usbfs.hpp"


/* Module variables. */

static absolute_time_t    m_next_ntp_check = nil_time;


/* Local / callback functions; not expected to be called from outside. */

/*
 * ntp_request - sends an NTP request to the server. 
 */

void time_ntp_request( uc_ntpstate_t *p_ntpstate )
{
  struct pbuf  *l_buffer;
  uint8_t      *l_payload; 

  /* Calls into lwIP need to be correctly locked. */
  cyw43_arch_lwip_begin();

  /* Allocate the packet buffer we'll send. */
  l_buffer = pbuf_alloc( PBUF_TRANSPORT, UC_NTP_PACKAGE_LEN, PBUF_RAM );
  l_payload = (uint8_t *)l_buffer->payload;

  /* Just set the flag in the start of that packet as a V3 client request. */
  memset( l_payload, 0, UC_NTP_PACKAGE_LEN );
  l_payload[0] = 0x1b;

  /* And send it. */
  udp_sendto( p_ntpstate->socket, l_buffer, &p_ntpstate->server, UC_NTP_PORT );

  /* Lastly free up the buffer. */
  pbuf_free( l_buffer );

  /* End of lwIP locked calls. */
  cyw43_arch_lwip_end();

  /* All done. */
  return;
}


/*
 * set_rtc_by_utc - sets the RTC to the provided time, applying our current
 *                  timezone appropriately.
 */

void time_set_rtc_by_utc( time_t p_utctime )
{
  datetime_t    l_datetime;
  struct tm    *l_tmstruct;

  /* Convert the time_t into a more useful structure. */
  l_tmstruct = gmtime( &p_utctime );

  /* Fill in the datetime struct. */
  l_datetime.year  = l_tmstruct->tm_year;
  l_datetime.month = l_tmstruct->tm_mon;
  l_datetime.day   = l_tmstruct->tm_mday;
  l_datetime.dotw  = l_tmstruct->tm_wday;
  l_datetime.hour  = l_tmstruct->tm_hour;
  l_datetime.min   = l_tmstruct->tm_min;
  l_datetime.sec   = l_tmstruct->tm_sec;

  /* And update the RTC. */
  rtc_set_datetime( &l_datetime );

  /* All done. */
  return;
}


/*
 * ntp_response_cb - callback function when an NTP response is received.
 */

void time_ntp_response_cb( void *p_state, struct udp_pcb *p_socket, 
                           struct pbuf *p_buffer, const ip_addr_t *p_addr,
                           uint16_t p_port )
{
  uc_ntpstate_t  *l_ntpstate = (uc_ntpstate_t *)p_state;
  uint8_t         l_mode, l_stratum;
  uint8_t         l_ntptime[4];

  /*
   * Called whenever we receive *any* packet; first step is to extract a few
   * important flags.
   */
  l_mode = pbuf_get_at( p_buffer, 0 ) & 0x07;
  l_stratum = pbuf_get_at( p_buffer, 1 );

  /*
   * And now we try to ensure that this is the data we expected, and not some
   * other random UDP packet. As long as it's the right size and port, we'll
   * try to make sense of it.
   */
  if ( ( p_port == UC_NTP_PORT ) && ( p_buffer->tot_len == UC_NTP_PACKAGE_LEN ) &&
       ( l_mode == 0x04 ) && ( l_stratum != 0 ) )
  {
    /* Looks valid; just extract the time value. */
    pbuf_copy_partial( p_buffer, l_ntptime, sizeof( l_ntptime ), 40 );
    l_ntpstate->ntptime = l_ntptime[0] << 24 | l_ntptime[1] << 16 | 
                          l_ntptime[2] << 8 | l_ntptime[3];
  }

  /* All done. */
  return;
}


/*
 * dns_response_cb - callback function when a DNS lookup completes.
 */

void time_dns_response_cb( const char *p_name, const ip_addr_t *p_addr,
                           void *p_state )
{
  uc_ntpstate_t  *l_ntpstate = (uc_ntpstate_t *)p_state;

  /* If the address is set to a value, we have our answer. */
  if ( p_addr != nullptr )
  {
    /* Save that address. */
    memcpy( &l_ntpstate->server, p_addr, sizeof( ip_addr_t ) );

    /* And initiate the request. */
    time_ntp_request( l_ntpstate );
  }
  else
  {
    /* Indicates a failure. All we can really do is set ourselve inactive. */
    usb_debug( "Failure in DNS callback" );
    l_ntpstate->active_query = false;
  }

  /* All done. */
  return;
}


/* Functions.*/

/*
 * init - sets up the time related things.
 */

void time_init( void )
{
  datetime_t l_time;

  /* Initialise the RTC */
  rtc_init();

  /* It also needs a valid time setting, before it runs. */
  l_time.year = 2023;
  l_time.month = 1;
  l_time.day = 1;
  l_time.dotw = 0;
  l_time.hour = l_time.min = l_time.sec = 0;
  rtc_set_datetime( &l_time );

  /* All done. */
  return;
}


/*
 * check_sync - if we haven't updated from NTP in some time, initiate it.
 *              returns true once we have either successfully updated, or
 *              failed in a fairly firm way.
 */

bool time_check_sync( const uc_config_t *p_config )
{
  static bool           l_connecting = false;
  static uc_ntpstate_t  l_ntpstate;
  int                   l_link_status, l_retval;
  time_t                l_utctime;

  /* If we're not due to sync, just say true right away. */
  if ( !time_reached( m_next_ntp_check ) )
  {
    return true;
  }

  /* So, we're due a check. First off, we need to bring the WiFi online. */

  /*
   * WiFi on PicoW is relatively painless; first off, we initialise the library,
   * switch to station mode and initiate the connection asynchronously.
   */
  if ( !l_connecting )
  {
    /* Initialise the network. */
    cyw43_arch_init();
    cyw43_arch_enable_sta_mode();
    cyw43_arch_wifi_connect_async( 
      p_config->wifi_ssid, p_config->wifi_password, CYW43_AUTH_WPA2_AES_PSK
    );

    /* Reset the state object we'll use for our NTP query. */
    if ( l_ntpstate.socket != nullptr )
    {
      udp_remove( l_ntpstate.socket );
      l_ntpstate.socket = nullptr;
    }
    l_ntpstate.ntptime = 0;
    l_ntpstate.active_query = false;

    /* Set a flag that we're waiting for the connection and return. */
    l_connecting = true;
    return false;
  }

  /*
   * So, the connecting flag is true so we're just busy waiting for the link 
   * status to change to 'link up'
   */
  l_link_status = cyw43_tcpip_link_status( &cyw43_state, CYW43_ITF_STA );

  /*
   * If the status is a hard fail, shut it all down and return true - this will
   * schedule another attempt at some point in the future, by which time with
   * any luck the problem has gone away!
   */
  if ( ( l_link_status == CYW43_LINK_FAIL ) || ( l_link_status == CYW43_LINK_BADAUTH ) ||
       ( l_link_status == CYW43_LINK_NONET ) )
  {
    usb_debug( "Failed to initialise WiFi (link status %d)", l_link_status );
    cyw43_arch_deinit();
    l_connecting = false;
    return true;
  }

  /*
   * If the link isn't up yet, that's not a failure - just means we need to
   * wait a little longer.
   */
  if ( l_link_status != CYW43_LINK_UP )
  {
    return false;
  }

  /*
   * So if we reach here, the WiFi link is up and available. Get the socket
   * that we need to work with.
   */
  if ( l_ntpstate.socket == nullptr )
  {
    l_ntpstate.socket = udp_new_ip_type( IPADDR_TYPE_ANY );
    if ( l_ntpstate.socket == nullptr )
    {
      usb_debug( "Failed to create UDP PCB socket" );
      return false;
    }

    /* Set up the callback to handle any packets received on this socket. */
    udp_recv( l_ntpstate.socket, time_ntp_response_cb, &l_ntpstate );
  }

  /*
   * If we don't have an active query, it means we need to start one. 
   */
  if ( !l_ntpstate.active_query )
  {
    /*
     * Step one is to look up the DNS address of our target NTP server. This
     * sort of low level operation needs to be properly gated with lwIP.
     */
    cyw43_arch_lwip_begin();
    l_ntpstate.active_query = true;
    l_retval = dns_gethostbyname( p_config->ntp_server, &l_ntpstate.server, 
                                  time_dns_response_cb, &l_ntpstate );
    cyw43_arch_lwip_end();

    /*
     * The DNS lookup may return immediately, if we already have a cached answer.
     */
    if ( l_retval == ERR_OK )
    {
      /* Then we can just raise the NTP request directly. */
      time_ntp_request( &l_ntpstate );
    }
    else if ( l_retval != ERR_INPROGRESS )
    {
      /* A DNS failure; call this a hard fail, and try next cycle. */
      usb_debug( "Failed to look up NTP server address (%d)", l_retval );
      cyw43_arch_deinit();
      l_connecting = false;
      return true;
    }

    /* Nothing more to do; the next cycle may find more work to do. */
    return false;
  }

  /*
   * So here, we have an active NTP query. The actual response will be caught
   * in callbacks, so here we just wait for NTP time value to appear in the
   * state structure. Once we have that, we apply it and close down the WiFi.
   */
  if ( l_ntpstate.ntptime > 0 )
  {
    /* Convert the NTP time into a Unix Epoch style value. */
    l_utctime = l_ntpstate.ntptime - UC_NTP_EPOCH_OFFSET;

    /* And just update the RTC clock appropriately. */
    time_set_rtc_by_utc( l_utctime );

    /* Schedule the next NTP sync for the future... */
    m_next_ntp_check = make_timeout_time_ms( UC_NTP_REFRESH_MS );

    /* That's it, we're all done! */
    cyw43_arch_deinit();
    l_connecting = false;
    return true;    
  }

  /* Not a failure - just means we have more work to do. */
  return false;
}


/* End of file time.cpp */
