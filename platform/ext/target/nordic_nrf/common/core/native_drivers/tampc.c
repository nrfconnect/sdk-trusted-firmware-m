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

void tampc_enable_interrupts(void)
{
	nrf_tampc_int_enable(NRF_TAMPC, NRF_TAMPC_ALL_INTS_MASK);
}

static void tampc_clear_events(void)
{
	nrf_tampc_event_clear(NRF_TAMPC, NRF_TAMPC_EVENT_TAMPER);
	nrf_tampc_event_clear(NRF_TAMPC, NRF_TAMPC_EVENT_WRITE_ERROR);
}

static void tampc_clear_statuses(void)
{
	/* The datasheet states that they detectors must be reset before the status is cleared. */
	nrf_tampc_protector_ctrl_value_set(NRF_TAMPC, NRF_TAMPC_PROTECT_CRACEN, false);
	nrf_tampc_protector_status_clear(NRF_TAMPC, NRF_TAMPC_PROTECT_CRACEN);

	nrf_tampc_protector_ctrl_value_set(NRF_TAMPC, NRF_TAMPC_PROTECT_GLITCH_DOMAIN_FAST, false);
	nrf_tampc_protector_status_clear(NRF_TAMPC, NRF_TAMPC_PROTECT_GLITCH_DOMAIN_FAST);

	nrf_tampc_protector_ctrl_value_set(NRF_TAMPC, NRF_TAMPC_PROTECT_GLITCH_DOMAIN_SLOW, false);
	nrf_tampc_protector_status_clear(NRF_TAMPC, NRF_TAMPC_PROTECT_GLITCH_DOMAIN_SLOW);
}

void tampc_configuration(void)
{

	tampc_clear_events();
	tampc_clear_statuses();

	/* Make sure that the CRACEN detector and the glitch detectors are enabled and lock
	 * their configuration.
	 */
	nrf_tampc_protector_ctrl_value_set(NRF_TAMPC, NRF_TAMPC_PROTECT_CRACEN, true);
	nrf_tampc_protector_ctrl_lock_set(NRF_TAMPC, NRF_TAMPC_PROTECT_CRACEN, true);

	nrf_tampc_protector_ctrl_value_set(NRF_TAMPC, NRF_TAMPC_PROTECT_GLITCH_DOMAIN_FAST, true);
	nrf_tampc_protector_ctrl_lock_set(NRF_TAMPC, NRF_TAMPC_PROTECT_GLITCH_DOMAIN_FAST, true);

	nrf_tampc_protector_ctrl_value_set(NRF_TAMPC, NRF_TAMPC_PROTECT_GLITCH_DOMAIN_SLOW, true);
	nrf_tampc_protector_ctrl_lock_set(NRF_TAMPC, NRF_TAMPC_PROTECT_GLITCH_DOMAIN_SLOW, true);

	/* The INTRESETEN reset value is enabled on reset, in order to use the TAMPC interrupt
	 * and not reset the device immediately the INTRESETEN is set to 0 here.
	 */
	nrf_tampc_protector_ctrl_value_set(NRF_TAMPC, NRF_TAMPC_PROTECT_RESETEN_INT, false);
	nrf_tampc_protector_ctrl_lock_set(NRF_TAMPC, NRF_TAMPC_PROTECT_RESETEN_INT, false);

	/* The active shield is not configured here yet, this will be added later. */
}

#endif
