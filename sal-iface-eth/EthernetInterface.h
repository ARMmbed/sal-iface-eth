/*
 * Copyright (c) 2012-2015, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ETHERNETINTERFACE_H_
#define ETHERNETINTERFACE_H_

#if !defined(TARGET_LPC1768) && !defined(TARGET_LPC4088) && !defined(TARGET_K64F) && !defined(TARGET_RZ_A1H) \
    && !defined(TARGET_LIKE_NUC472)
#error The Ethernet Interface library is not supported on this target
#endif

#include "lwip/netif.h"
#include "minar/minar.h"
#include "core-util/FunctionPointer.h"

 /** Interface using Ethernet to connect to an IP-based network
 *
 */
class EthernetInterface {
public:
  typedef mbed::util::FunctionPointer3<void, EthernetInterface *, bool, bool> StatusChangeHandler_t;
  EthernetInterface();
  /** Initialize the interface with DHCP.
  * Initialize the interface and configure it to use DHCP (no connection at this point).
  * \return 0 on success, a negative number on failure
  */
  int init(); //With DHCP

  /** Initialize the interface with a IP address.
  * Initialize the interface and configure it with the following configuration (no connection at this point).
  * \param ip the IP address to use
  * \param mask the IP address mask
  * \param gateway the gateway to use
  * \return 0 on success, a negative number on failure
  */
  int init(const char* ip, const char* mask, const char* gateway);

  /** Connect
  * Bring the interface up, start DHCP if needed.
  * \param   timeout_ms  timeout in ms (default: (15)s).
  */
  void connect(StatusChangeHandler_t handler, unsigned int timeout_ms=15000);

  /** Disconnect
  * Bring the interface down
  * \return 0 on success, a negative number on failure
  */
  void disconnect();

  /** Get the MAC address of your Ethernet interface
   * \return a pointer to a string containing the MAC address
   */
  char* getMACAddress();

  /** Get the IP address of your Ethernet interface
   * \return a pointer to a string containing the IP address
   */
  char* getIPAddress();

  /** Get the Gateway address of your Ethernet interface
   * \return a pointer to a string containing the Gateway address
   */
  char* getGateway();

  /** Get the Network mask of your Ethernet interface
   * \return a pointer to a string containing the Network mask
   */
  char* getNetworkMask();
  void netif_status_callback();
  void netif_link_callback();
  struct netif netif;
protected:
  void connect_abort();
  void init_netif(ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw);
protected:

  char mac_addr[19];
  char ip_addr[17];
  char gateway[17];
  char networkmask[17];
  bool use_dhcp = false;

  volatile int8_t link_up;
  volatile int8_t if_up;

  minar::callback_handle_t _timeoutHandle;
  StatusChangeHandler_t _handler;
};

#endif /* ETHERNETINTERFACE_H_ */
