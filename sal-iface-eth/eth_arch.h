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

// Architecture specific Ethernet interface
// Must be implemented by each target

#ifndef ETHARCH_H_
#define ETHARCH_H_

#include "lwip/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

void eth_arch_enable_interrupts(void);
void eth_arch_disable_interrupts(void);
err_t eth_arch_enetif_init(struct netif *netif);
void eth_arch_timer_callback(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef ETHARCHINTERFACE_H_

