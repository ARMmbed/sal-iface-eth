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

#include "EthernetInterface.h"

#include "lwip/inet.h"
#include "lwip/netif.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "eth_arch.h"
#include "lwip/tcpip.h"
#include "lwip/init.h"

#include "mbed-drivers/mbed.h"
#include "minar/minar.h"
#include <stdint.h>

/* TCP/IP and Network Interface Initialisation */
extern volatile uint8_t allow_net_callbacks;

const uintptr_t netif_offset = (uintptr_t)&(((EthernetInterface*)8)->netif) - (uintptr_t)8;

// Dangerous!
void status_callback(struct netif *netif) {
    EthernetInterface * eth = (EthernetInterface *)((uintptr_t)netif - netif_offset);
    eth->netif_status_callback();
}
void link_callback(struct netif *netif) {
    EthernetInterface * eth = (EthernetInterface *)((uintptr_t)netif - netif_offset);
    eth->netif_link_callback();
}
void EthernetInterface::netif_link_callback() {
    bool linkUp = netif_is_link_up(&netif);
    bool ifUp   = netif_is_up(&netif);
    printf("linkcb: link %s, if %s\r\n", linkUp?"up":"down", ifUp?"up":"down");
    minar::Scheduler::postCallback(_handler.bind(this, linkUp, ifUp));
    if (_timeoutHandle) {
        minar::Scheduler::cancelCallback(_timeoutHandle);
        _timeoutHandle = minar::callback_handle_t();
    }
}

void EthernetInterface::netif_status_callback() {
    bool linkUp = netif_is_link_up(&netif);
    bool ifUp   = netif_is_up(&netif);
    printf("statuscb: link %s, if %s\r\n", linkUp?"up":"down", ifUp?"up":"down");
    if (ifUp) {
        strcpy(ip_addr, inet_ntoa(netif.ip_addr));
        strcpy(gateway, inet_ntoa(netif.gw));
        strcpy(networkmask, inet_ntoa(netif.netmask));
    }
    minar::Scheduler::postCallback(_handler.bind(this, linkUp, ifUp));
    if (_timeoutHandle) {
        minar::Scheduler::cancelCallback(_timeoutHandle);
        _timeoutHandle = minar::callback_handle_t();
    }
}

void EthernetInterface::init_netif(ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw) {
    lwip_init();

    memset((void*) &netif, 0, sizeof(netif));
    netif_add(&netif, ipaddr, netmask, gw, NULL, eth_arch_enetif_init, ethernet_input);
    netif_set_default(&netif);

    netif_set_link_callback  (&netif, link_callback);
    netif_set_status_callback(&netif, status_callback);

    allow_net_callbacks = 1;
}

void set_mac_address(char * mac_addr) {
#if (MBED_MAC_ADDRESS_SUM != MBED_MAC_ADDR_INTERFACE)
    snprintf(mac_addr, 19, "%02x:%02x:%02x:%02x:%02x:%02x", MBED_MAC_ADDR_0, MBED_MAC_ADDR_1, MBED_MAC_ADDR_2,
             MBED_MAC_ADDR_3, MBED_MAC_ADDR_4, MBED_MAC_ADDR_5);
#else
    char mac[6];
    mbed_mac_address(mac);
    snprintf(mac_addr, 19, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
}

EthernetInterface::EthernetInterface() :
    use_dhcp(false)
{
    ip_addr[0] = gateway[0] = networkmask[0] = '\0';
}

int EthernetInterface::init() {
    use_dhcp = true;
    set_mac_address(mac_addr);
    init_netif(NULL, NULL, NULL);
    return 0;
}

int EthernetInterface::init(const char* ip, const char* mask, const char* gateway) {
    use_dhcp = false;

    set_mac_address(mac_addr);
    strcpy(ip_addr, ip);

    ip_addr_t ip_n, mask_n, gateway_n;
    inet_aton(ip, &ip_n);
    inet_aton(mask, &mask_n);
    inet_aton(gateway, &gateway_n);
    init_netif(&ip_n, &mask_n, &gateway_n);

    return 0;
}

void EthernetInterface::connect(StatusChangeHandler_t handler, unsigned int timeout_ms) {
    eth_arch_enable_interrupts();
    _timeoutHandle = minar::Scheduler::postCallback(this,&EthernetInterface::disconnect)
        .delay(minar::milliseconds(timeout_ms))
        .getHandle();
    _handler = handler;
    if (use_dhcp) {
        dhcp_start(&netif);

        // Wait for an IP Address
        // -1: error, 0: timeout
        // while (if_up == 0);
    } else {
        netif_set_up(&netif);
        // while (link_up == 0);
    }
    // _timeout.detach();
    //
    // if (link_up < 0 || if_up < 0) {
    //     return -1;
    // } else if (link_up == 0 && if_up == 0) {
    //     return -1;
    // } else {
    //     return 0;
    // }
}

void EthernetInterface::disconnect() {
    if (use_dhcp) {
        dhcp_release(&netif);
        dhcp_stop(&netif);
    } else {
        netif_set_down(&netif);
    }

    eth_arch_disable_interrupts();
    if_up = link_up = 0;
}

char* EthernetInterface::getMACAddress() {
    return mac_addr;
}

char* EthernetInterface::getIPAddress() {
    return ip_addr;
}

char* EthernetInterface::getGateway() {
    return gateway;
}

char* EthernetInterface::getNetworkMask() {
    return networkmask;
}
