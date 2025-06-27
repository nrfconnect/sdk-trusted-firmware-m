/*
 * Copyright (c) 2025 Nordic Semiconductor ASA.
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#ifndef __TAMPC_H__
#define __TAMPC_H__


#include "target_cfg.h"
#include <nrfx.h>
#include <hal/nrf_tampc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * \brief Enable TAMPC interrupts
 *
 * Enable interrupts in the INTENSET register of TAMPC.
 */
void tampc_enable_interrupts(void);

/**
 * \brief TAMPC configuration
 *
 * Configures the TAMPC peripheral to monitor for Cracen and slow/fast domain hardware
 * attacks and disables the default reset behavior in order to hanlde the event in the
 * interrupt handler. It also locks the configuration of the CTRL registers
 * so that they cannot be altered until reset.
 * 
 */
void tampc_configuration(void);

#endif
