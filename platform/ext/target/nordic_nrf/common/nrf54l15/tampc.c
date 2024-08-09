
/***


*/

#include "nrf54l15_enga_application.h"
#include "nrf54l15_enga_global.h"
#include "nrf54l15_enga_types.h"
#include "tfm_hal_platform.h"
#include "tfm_plat_defs.h"
#include <hal/nrf_tampc.h>
#include <helpers/nrfx_reset_reason.h>

bool boot_reason_sectamper;

#define RESET_BY_PERIPHERAL false
#define TAMPER_EVENT_HANDLER tfm_core_panic

#define TAMPC_REASON (((volatile uint32_t *)0x4010E600)[0])
void tampc_enable(void);
void tampc_disable(void);

int init_tampc(void) {

  if (TAMPC_REASON & NRFX_RESET_REASON_SECTAMPER_MASK) {
    TAMPC_REASON = NRFX_RESET_REASON_SECTAMPER_MASK;
    boot_reason_sectamper = true;
  }

  nrf_tampc_event_clear(NRF_TAMPC, NRF_TAMPC_EVENT_TAMPER);
  nrf_tampc_event_clear(NRF_TAMPC, NRF_TAMPC_EVENT_WRITE_ERROR);

  tampc_enable();
  nrf_tampc_int_enable(NRF_TAMPC, NRF_TAMPC_ALL_INTS_MASK);

  nrf_tampc_protector_ctrl_value_set(NRF_TAMPC, NRF_TAMPC_PROTECT_RESETEN_INT,
                                     RESET_BY_PERIPHERAL);

  NVIC_ClearPendingIRQ(TAMPC_IRQn);
  NVIC_ClearTargetState(TAMPC_IRQn);
  NVIC_EnableIRQ(TAMPC_IRQn);

  return TFM_PLAT_ERR_SUCCESS;
}

void tampc_enable(void) {
  nrf_tampc_protector_ctrl_value_set(
      NRF_TAMPC, NRF_TAMPC_PROTECT_GLITCH_DOMAIN_FAST, true);
  nrf_tampc_protector_ctrl_value_set(
      NRF_TAMPC, NRF_TAMPC_PROTECT_GLITCH_DOMAIN_SLOW, true);
}

void tampc_disable(void) {
  nrf_tampc_protector_ctrl_value_set(
      NRF_TAMPC, NRF_TAMPC_PROTECT_GLITCH_DOMAIN_FAST, false);
  nrf_tampc_protector_ctrl_value_set(
      NRF_TAMPC, NRF_TAMPC_PROTECT_GLITCH_DOMAIN_SLOW, false);
}

void TAMPC_Handler(void) {
  SPMLOG_ERRMSG("TAMPC Exception:\r\n");

  if (nrf_tampc_event_check(NRF_TAMPC, NRF_TAMPC_EVENT_WRITE_ERROR)) {
    SPMLOG_ERRMSG("\tWrite error.\r\n");
    nrf_tampc_event_clear(NRF_TAMPC, NRF_TAMPC_EVENT_WRITE_ERROR);
    NVIC_ClearPendingIRQ(TAMPC_IRQn);
    tfm_core_panic();
  }

  if (nrf_tampc_event_check(NRF_TAMPC, NRF_TAMPC_EVENT_TAMPER)) {
    SPMLOG_ERRMSG("\tTamper event.\r\n");
    nrf_tampc_event_clear(NRF_TAMPC, NRF_TAMPC_EVENT_TAMPER);
    NVIC_ClearPendingIRQ(TAMPC_IRQn);
    TAMPER_EVENT_HANDLER();
  }
}
