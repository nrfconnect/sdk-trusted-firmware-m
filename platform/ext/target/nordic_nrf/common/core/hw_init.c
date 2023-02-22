/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
#include <nrf.h>
#include "array.h"

/* This routine clears all ARM MPU region configuration. */
static void hw_init_mpu(void)
{
	const int num_regions =
		((MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos);

	for (int i = 0; i < num_regions; i++) {
		ARM_MPU_ClrRegion(i);
	}
}

/* This routine resets Cortex-M system control block components and core
 * registers.
 */
void hw_init_reset_on_boot(void)
{
	/* Disable interrupts */
	__disable_irq();

	/* Reset exception and interrupt mask state (PRIMASK handled by
	 * __enable_irq below)
	 */
	__set_FAULTMASK(0);
	__set_BASEPRI(0);

	/* Clear MPU region configuration */
	hw_init_mpu();

	/* Disable NVIC interrupts */
	for (int i = 0; i < ARRAY_SIZE(NVIC->ICER); i++) {
		NVIC->ICER[i] = 0xFFFFFFFF;
	}
	/* Clear pending NVIC interrupts */
	for (int i = 0; i < ARRAY_SIZE(NVIC->ICPR); i++) {
		NVIC->ICPR[i] = 0xFFFFFFFF;
	}

	/* Restore Interrupts */
	__enable_irq();

	__DSB();
	__ISB();
}
