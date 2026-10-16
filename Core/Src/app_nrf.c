/**
 ******************************************************************************
 * @file    app_nrf.c
 * @brief   nRF24L01+ RX profile selector for the main board.
 *
 * @details Ten pre-shared profiles are available.  Each profile defines:
 *            - a 5-byte pipe address (RX pipe 1)
 *            - an RF channel number
 *            - a system_id that the received payload must match
 *
 *          The sensor node (transmitter) must be configured with the SAME
 *          profile index so that both ends agree on address, channel and
 *          system_id.  If they mismatch, no packet will be accepted.
 *
 * @note    The RX IRQ handler reads the payload, checks system_id and
 *          only then pushes it into the FreeRTOS queue.
 ******************************************************************************
 */

#include "app_nrf.h"
#include "app_config.h"
#include "app_globals.h"
#include "nrf24.h"
#include <math.h>           /* Required for powf() */

/*==============================================================================
 *                          RF PROFILE SELECTOR
 *============================================================================*/
/**
 * @brief  Select one of ten pre-shared RF profiles and apply it to the radio.
 * @param  mode  Valid range 1..10; out-of-range values fall back to mode 1.
 *
 * @note   This function is intentionally implemented as a switch statement
 *         so that every profile is visible at a glance and no dynamic
 *         memory is used.  Each case writes:
 *           - addr[0..4] : 5-byte pipe address
 *           - RFChannel  : RF channel number
 *           - system_id  : network identifier expected from the sender
 */
void apply_mode(uint8_t mode)
{
    uint8_t addr[5];       /* 5-byte pipe address buffer (on stack) */
    uint8_t RFChannel;     /* selected RF channel number            */

    switch (mode)
    {
    /*----- Profile 1: default network -------------------------------------*/
    case 1U:
        addr[0] = 0xA1U;
        addr[1] = 0xB2U;
        addr[2] = 0xC3U;
        addr[3] = 0xD4U;
        addr[4] = 0xE5U;
        RFChannel = 76U;
        system_id = 0x6FU;
        break;

    /*----- Profile 2 ------------------------------------------------------*/
    case 2U:
        addr[0] = 0x67U;
        addr[1] = 0x89U;
        addr[2] = 0xABU;
        addr[3] = 0xCDU;
        addr[4] = 0xEFU;
        RFChannel = 93U;
        system_id = 0x10U;
        break;

    /*----- Profile 3 ------------------------------------------------------*/
    case 3U:
        addr[0] = 0x12U;
        addr[1] = 0x34U;
        addr[2] = 0x56U;
        addr[3] = 0x78U;
        addr[4] = 0x9AU;
        RFChannel = 27U;
        system_id = 0xBCU;
        break;

    /*----- Profile 4 ------------------------------------------------------*/
    case 4U:
        addr[0] = 0x0FU;
        addr[1] = 0x1EU;
        addr[2] = 0x2DU;
        addr[3] = 0x3CU;
        addr[4] = 0x4BU;
        RFChannel = 35U;
        system_id = 0x59U;
        break;

    /*----- Profile 5 ------------------------------------------------------*/
    case 5U:
        addr[0] = 0x5AU;
        addr[1] = 0x6BU;
        addr[2] = 0x7CU;
        addr[3] = 0x8DU;
        addr[4] = 0x9EU;
        RFChannel = 36U;
        system_id = 0x0CU;
        break;

    /*----- Profile 6 ------------------------------------------------------*/
    case 6U:
        addr[0] = 0x11U;
        addr[1] = 0x22U;
        addr[2] = 0x33U;
        addr[3] = 0x44U;
        addr[4] = 0x55U;
        RFChannel = 40U;
        system_id = 0xDEU;
        break;

    /*----- Profile 7 ------------------------------------------------------*/
    case 7U:
        addr[0] = 0xAAU;
        addr[1] = 0xBBU;
        addr[2] = 0xCCU;
        addr[3] = 0xDDU;
        addr[4] = 0xEEU;
        RFChannel = 67U;
        system_id = 0x13U;
        break;

    /*----- Profile 8 ------------------------------------------------------*/
    case 8U:
        addr[0] = 0x01U;
        addr[1] = 0x23U;
        addr[2] = 0x45U;
        addr[3] = 0x67U;
        addr[4] = 0x89U;
        RFChannel = 26U;
        system_id = 0xFAU;
        break;

    /*----- Profile 9 ------------------------------------------------------*/
    case 9U:
        addr[0] = 0xF0U;
        addr[1] = 0xE1U;
        addr[2] = 0xD2U;
        addr[3] = 0xC3U;
        addr[4] = 0xB4U;
        RFChannel = 65U;
        system_id = 0x08U;
        break;

    /*----- Profile 10 -----------------------------------------------------*/
    case 10U:
        addr[0] = 0x9EU;
        addr[1] = 0x8DU;
        addr[2] = 0x7CU;
        addr[3] = 0x6BU;
        addr[4] = 0x5AU;
        RFChannel = 2U;
        system_id = 0x37U;
        break;

    /*----- Fallback: same as profile 1 -----------------------------------*/
    default:
        addr[0] = 0xA1U;
        addr[1] = 0xB2U;
        addr[2] = 0xC3U;
        addr[3] = 0xD4U;
        addr[4] = 0xE5U;
        RFChannel = 76U;
        system_id = 0x6FU;
        break;
    }

    /* Apply the selected profile to the radio. */
    nRF24_SetRFChannel(RFChannel);
    nRF24_SetAddr(nRF24_PIPE1, addr);
    nRF24_FlushTX();
    nRF24_FlushRX();
    nRF24_ClearIRQFlags();

    /* Re-arm the radio in RX mode with the new profile. */
    nRF24_CE_L();
    nRF24_SetOperationalMode(nRF24_MODE_RX);
    nRF24_CE_H();
}

/*==============================================================================
 *                          DERIVED LDR VALUE (LUX)
 *============================================================================*/
/**
 * @brief  Convert the raw LDR ADC value into an approximate lux reading.
 * @note   R_LDR is derived from the standard voltage-divider equation with
 *         the 10 kOhm load resistor.  Lux uses a power-law fit.
 */
static void AppNrf_ComputeLux(void)
{
    LDR_Vout = ((float)rxData.ldr / ADC_MAX) * V_REF;

    if (LDR_Vout > 0.001f)
    {
        R_LDR = (RL * (V_REF - LDR_Vout)) / LDR_Vout;
    }
    else
    {
        R_LDR = 2000000.0f;    /* Saturated dark reading */
    }

    if (R_LDR > 0.0f)
    {
        lux = 12518931.0f * powf(R_LDR, -1.405f);
    }
    else
    {
        lux = 0.0f;
    }

    if (lux > 10000.0f) { lux = 10000.0f; }
    if (lux < 0.5f)     { lux = 0.0f;     }
}

/*==============================================================================
 *                          DERIVED MQ-135 VALUE (PPM)
 *============================================================================*/
/**
 * @brief  Convert the raw MQ-135 ADC value into an NH3 concentration in ppm.
 * @note   The sensor is powered from 5 V, but the ADC reads a divided signal
 *         referenced to V_REF, hence the (5 / V_REF) rescaling step.
 */
static void AppNrf_ComputePpm(void)
{
    R0_MQ135 = 7000.0f;

    MQ135_Vout  = ((float)rxData.gas / ADC_MAX) * V_REF;
    Vout_sensor = MQ135_Vout * (5.0f / V_REF);

    if (MQ135_Vout > 0.001f)
    {
        R_MQ135 = ((5.0f * RL) / Vout_sensor) - RL;
    }
    else
    {
        MQ135_Vout = 0.0f;
    }

    if (R0_MQ135 > 0.0f)
    {
        ratio = R_MQ135 / R0_MQ135;
    }
    else
    {
        ratio = 0.0f;
    }

    if (ratio > 0.0f)
    {
        ppm = MQ135_A_NH3 * powf(ratio, MQ135_B_NH3);
    }
    else
    {
        ppm = 0.0f;
    }

    /* Clamp to the useful datasheet range. */
    if (ppm < 10.0f)   { ppm = 0.0f;    }
    if (ppm > 1000.0f) { ppm = 1000.0f; }
}

/*==============================================================================
 *                          RECEIVER TASK
 *============================================================================*/
/**
 * @brief  FreeRTOS task: consume the RX queue and refresh derived sensor values.
 * @param  pvParameters  Unused (NULL).
 *
 * @note   The task blocks on the queue for up to 100 ms.  When a payload
 *         arrives, it marks the data as fresh and resets the timeout
 *         counter.  When nothing arrives for 200 ticks (10 s at 50 ms/tick),
 *         the "no sensor" flag is raised so the LCD can display it.
 *
 *         After each queue access, the LDR and MQ-135 raw values are
 *         converted into engineering units (lux and ppm).
 */
void Task_nRF_Receiver(void *pvParameters)
{
    (void)pvParameters;

    static TickType_t fixed_delay       = 0U;
    static uint16_t   no_sensor_counter = 0U;

    for (;;)
    {
        /* -------- Pull one payload from the ISR queue, if any -------- */
        if (xQueueReceive(nrfDataR_Queue, &rxData, 100U) == pdPASS)
        {
            new_data_available = 1U;
            no_sensor_counter  = 0U;
        }
        else
        {
            vTaskDelayUntil(&fixed_delay, pdMS_TO_TICKS(50U));
            no_sensor_counter++;
        }

        if (no_sensor_counter >= 200U)
        {
            new_data_available = 0U;
            no_sensor_counter  = 200U;
        }

        /* -------- Refresh derived values -------- */
        AppNrf_ComputeLux();
        AppNrf_ComputePpm();
    }
}
