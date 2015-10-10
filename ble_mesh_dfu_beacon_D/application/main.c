/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_app_proximity_main main.c
 * @{
 * @ingroup ble_sdk_app_proximity_eval
 * @brief Proximity Application main file.
 *
 * This file contains is the source code for a sample proximity application using the
 * Immediate Alert, Link Loss and Tx Power services.
 *
 * This application would accept pairing requests from any peer device.
 *
 * It demonstrates the use of fast and slow advertising intervals.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "app_error.h"
#include "nrf51_bitfields.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "ble_sensorsim.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "device_manager.h"
#include "app_gpiote.h"
#include "app_util.h"
#include "pstorage.h"
#include "app_trace.h"
#include "bsp.h"
//add files for ble_mesh
#include "timeslot_handler.h"
#include "rbc_mesh.h"
#include "rbc_mesh_common.h"

//add files for seelight
#include "pwm.h"
#include "dispose_pkt.h"

//add dfu 
#ifdef BLE_DFU_APP_SUPPORT
#include "ble_dfu.h"
#include "dfu_app_handler.h"
#endif // BLE_DFU_APP_SUPPORT


//add beacon{
#define APP_BEACON_INFO_LENGTH        0x17                              /**< Total length of information advertised by the Beacon. */
#define APP_ADV_DATA_LENGTH           0x15                              /**< Length of manufacturer specific data in the advertisement. */
#define APP_DEVICE_TYPE               0x02                              /**< 0x02 refers to Beacon. */
#define APP_COMPANY_IDENTIFIER        0x004C                            /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */
//Big-endian
#define APP_BEACON_UUID               0xFD, 0xA5, 0x06, 0x93, 0xA4, 0xE2, 0x4F, 0xB1, 0xAF, 0xCF, 0xC6, 0xEB,  0x07, 0x64, 0x78, 0x25
#define APP_MAJOR_VALUE               0x27, 0x11                        /**< Major value used to identify Beacons. */ 
#define APP_MINOR_VALUE               0x30, 0x39                        /**< Minor value used to identify Beacons. */ 
#define APP_MEASURED_RSSI             0xC8                              /**< The Beacon's measured RSSI at 1 meter distance in dBm. */

#define DEAD_BEEF                     0xDEADBEEF                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

//}

#define IS_SRVC_CHANGED_CHARACT_PRESENT   0                                                 /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define SIGNAL_ALERT_BUTTON_ID            0                                                 /**< Button used for send or cancel High Alert to the peer. */
#define WAKEUP_BUTTON_ID                  0                                                 /**< Button used to wakeup MCU from power off mode */
#define STOP_ALERTING_BUTTON_ID           1                                                 /**< Button used for clearing the Alert LED that may be blinking or turned ON because of alerts from the central. */
#define BOND_DELETE_ALL_BUTTON_ID         1                                                 /**< Button used for deleting all bonded centrals during startup. */

#define DEVICE_NAME                       "HS_Prox"                                     /**< Name of device. Will be included in the advertising data. */
#define APP_ADV_INTERVAL_FAST             0x0290                                            /**< Fast advertising interval (in units of 0.625 ms. This value corresponds to 250 ms.). */
#define APP_ADV_INTERVAL_SLOW             0x0C80                                            /**< Slow advertising interval (in units of 0.625 ms. This value corresponds to 2 seconds). */
#define APP_SLOW_ADV_TIMEOUT              180                                               /**< The duration of the slow advertising period (in seconds). */
#define APP_FAST_ADV_TIMEOUT              100//30                                           /**< The duration of the fast advertising period (in seconds). */

#define APP_TIMER_PRESCALER               0                                                 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS              (3+BSP_APP_TIMERS_NUMBER)                         /**< Maximum number of simultaneously created timers. 1 for Battery measurement, 2 for flashing Advertising LED and Alert LED, 1 for connection parameters module, 1 for button polling timer needed by the app_button module,  */
#define APP_TIMER_OP_QUEUE_SIZE           6                                                 /**< Size of timer operation queues. */

#define BATTERY_LEVEL_MEAS_INTERVAL       APP_TIMER_TICKS(120000, APP_TIMER_PRESCALER)      /**< Battery level measurement interval (ticks). This value corresponds to 120 seconds. */

#define ADV_LED_ON_TIME                   APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)         /**< Advertisement LED ON period when in blinking state. */
#define ADV_LED_OFF_TIME                  APP_TIMER_TICKS(900, APP_TIMER_PRESCALER)         /**< Advertisement LED OFF period when in blinking state. */

#define MILD_ALERT_LED_ON_TIME            APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)         /**< Alert LED ON period when in blinking state. */
#define MILD_ALERT_LED_OFF_TIME           APP_TIMER_TICKS(900, APP_TIMER_PRESCALER)         /**< Alert LED OFF period when in blinking state. */

#define MIN_CONN_INTERVAL                 MSEC_TO_UNITS(200, UNIT_1_25_MS)                  /**< Minimum acceptable connection interval (0.2 seconds).  */
#define MAX_CONN_INTERVAL                 MSEC_TO_UNITS(400, UNIT_1_25_MS)                 /**< Maximum acceptable connection interval (0.4 second). */
#define SLAVE_LATENCY                     0                                                 /**< Slave latency. */
#define CONN_SUP_TIMEOUT                  MSEC_TO_UNITS(4000, UNIT_10_MS)                   /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)        /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY     APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)       /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT      3                                                 /**< Number of attempts before giving up the connection parameter negotiation. */

#define APP_GPIOTE_MAX_USERS              1                                                 /**< Maximum number of users of the GPIOTE handler. */

#define BUTTON_DETECTION_DELAY            APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)          /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define SEC_PARAM_TIMEOUT                 30                                                /**< Timeout for Pairing Request or Security Request (in seconds). */
#define SEC_PARAM_BOND                    1                                                 /**< Perform bonding. */
#define SEC_PARAM_MITM                    0                                                 /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES         BLE_GAP_IO_CAPS_NONE                              /**< No I/O capabilities. */
#define SEC_PARAM_OOB                     0                                                 /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE            7                                                 /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE            16                                                /**< Maximum encryption key size. */

#define INITIAL_LLS_ALERT_LEVEL           BLE_CHAR_ALERT_LEVEL_NO_ALERT                     /**< Initial value for the Alert Level characteristic in the Link Loss service. */
#define TX_POWER_LEVEL                    4//(-8)                                              /**< TX Power Level value. This will be set both in the TX Power service, in the advertising data, and also used to set the radio transmit power. */

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS     1200                                              /**< Reference voltage (in milli volts) used by ADC while doing conversion. */
#define ADC_PRE_SCALING_COMPENSATION      3                                                 /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS    270                                               /**< Typical forward voltage drop of the diode (Part no: SD103ATW-7-F) that is connected in series with the voltage supply. This is the voltage drop when the forward current is 1mA. Source: Data sheet of 'SURFACE MOUNT SCHOTTKY BARRIER DIODE ARRAY' available at www.diodes.com. */

#define DEAD_BEEF                         0xDEADBEEF                                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

//add dfu
#ifdef BLE_DFU_APP_SUPPORT
#define DFU_REV_MAJOR                        0x00                                       /** DFU Major revision number to be exposed. */
#define DFU_REV_MINOR                        0x01                                       /** DFU Minor revision number to be exposed. */
#define DFU_REVISION                         ((DFU_REV_MAJOR << 8) | DFU_REV_MINOR)     /** DFU Revision number to be exposed. Combined of major and minor versions. */
#endif // BLE_DFU_APP_SUPPORT


/**@brief Macro to convert the result of ADC conversion in millivolts.
 *
 * @param[in]  ADC_VALUE   ADC result.
 * @retval     Result converted to millivolts.
 */
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / 255) * ADC_PRE_SCALING_COMPENSATION)

/**@brief Advertisement states. */
typedef enum
{
    BLE_NO_ADV,                                                                             /**< No advertising running. */
    BLE_FAST_ADV_WHITELIST,                                                                 /**< Advertising with whitelist. */
    BLE_FAST_ADV,                                                                           /**< Fast advertising running. */
    BLE_SLOW_ADV,                                                                           /**< Slow advertising running. */
    BLE_SLEEP,                                                                              /**< Go to system-off. */
} ble_advertising_mode_t;

static uint16_t                           m_conn_handle = BLE_CONN_HANDLE_INVALID;
//static ble_tps_t                          m_tps;                                            /**< Structure used to identify the TX Power service. */
//static ble_ias_t                          m_ias;                                            /**< Structure used to identify the Immediate Alert service. */
//static ble_lls_t                          m_lls;                                            /**< Structure used to identify the Link Loss service. */

//static ble_bas_t                          m_bas;                                            /**< Structure used to identify the battery service. */
//static ble_ias_c_t                        m_ias_c;                                          /**< Structure used to identify the client to the Immediate Alert Service at peer. */

static uint8_t                            m_advertising_mode;                               /**< Variable to keep track of when we are advertising. */

static volatile bool                      m_is_high_alert_signalled;                        /**< Variable to indicate whether or not high alert is signalled to the peer. */

static app_timer_id_t                     m_battery_timer_id;                               /**< Battery measurement timer. */
static dm_application_instance_t          m_app_handle;                                     /**< Application identifier allocated by device manager */

static bool                               m_memory_access_in_progress = false;              /**< Flag to keep track of ongoing operations on persistent memory. */

//static void on_ias_evt(ble_ias_t * p_ias, ble_ias_evt_t * p_evt);
//static void on_lls_evt(ble_lls_t * p_lls, ble_lls_evt_t * p_evt);
//static void on_ias_c_evt(ble_ias_c_t * p_lls, ble_ias_c_evt_t * p_evt);
//static void on_bas_evt(ble_bas_t * p_bas, ble_bas_evt_t * p_evt);
static void advertising_init(uint8_t adv_flags);

#ifdef BLE_DFU_APP_SUPPORT    
static ble_dfu_t                             m_dfus;                                    /**< Structure used to identify the DFU service. */
#endif // BLE_DFU_APP_SUPPORT    


//add beacon{
static uint8_t m_beacon_info[APP_BEACON_INFO_LENGTH] = {
    APP_DEVICE_TYPE,
    APP_ADV_DATA_LENGTH,
    APP_BEACON_UUID,
    APP_MAJOR_VALUE,
    APP_MINOR_VALUE,
    APP_MEASURED_RSSI
};
//}

extern dev_name_t     local_dev_name;
/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


//add dfu
#ifdef BLE_DFU_APP_SUPPORT    
static void advertising_stop(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_stop();
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
}


/** @snippet [DFU BLE Reset prepare] */
static void reset_prepare(void)
{
    uint32_t err_code;
    
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        // Disconnect from peer.
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        err_code = bsp_indication_set(BSP_INDICATE_IDLE);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        // If not connected, then the device will be advertising. Hence stop the advertising.
        advertising_stop();
    }

    err_code = ble_conn_params_stop();
    APP_ERROR_CHECK(err_code);
}
/** @snippet [DFU BLE Reset prepare] */
#endif // BLE_DFU_APP_SUPPORT    


/**@brief Function for handling the ADC interrupt.
 * @details  This function will fetch the conversion result from the ADC, convert the value into
 *           percentage and send it to peer.
 */
void ADC_IRQHandler(void)
{
//    if (NRF_ADC->EVENTS_END != 0)
//    {
//        uint8_t  adc_result;
//        uint16_t batt_lvl_in_milli_volts;
//        uint8_t  percentage_batt_lvl;
//        uint32_t err_code;

//        NRF_ADC->EVENTS_END = 0;
//        adc_result          = NRF_ADC->RESULT;
//        NRF_ADC->TASKS_STOP = 1;

//        batt_lvl_in_milli_volts = ADC_RESULT_IN_MILLI_VOLTS(adc_result) +
//                                  DIODE_FWD_VOLT_DROP_MILLIVOLTS;
//        percentage_batt_lvl     = battery_level_in_percent(batt_lvl_in_milli_volts);

//        err_code = ble_bas_battery_level_update(&m_bas, percentage_batt_lvl);
//        if (
//            (err_code != NRF_SUCCESS)
//            &&
//            (err_code != NRF_ERROR_INVALID_STATE)
//            &&
//            (err_code != BLE_ERROR_NO_TX_BUFFERS)
//            &&
//            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
//            )
//        {
//            APP_ERROR_HANDLER(err_code);
//        }
//    }
}


/**@brief Function for making the ADC start a battery level conversion.
 */
static void adc_start(void)
{
    uint32_t err_code;

    // Configure ADC
    NRF_ADC->INTENSET   = ADC_INTENSET_END_Msk;
    NRF_ADC->CONFIG     = (ADC_CONFIG_RES_8bit                        << ADC_CONFIG_RES_Pos)     |
                          (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos)  |
                          (ADC_CONFIG_REFSEL_VBG                      << ADC_CONFIG_REFSEL_Pos)  |
                          (ADC_CONFIG_PSEL_Disabled                   << ADC_CONFIG_PSEL_Pos)    |
                          (ADC_CONFIG_EXTREFSEL_None                  << ADC_CONFIG_EXTREFSEL_Pos);
    NRF_ADC->EVENTS_END = 0;
    NRF_ADC->ENABLE     = ADC_ENABLE_ENABLE_Enabled;

    // Enable ADC interrupt
    err_code = sd_nvic_ClearPendingIRQ(ADC_IRQn);
    APP_ERROR_CHECK(err_code);

    err_code = sd_nvic_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_LOW);
    APP_ERROR_CHECK(err_code);

    err_code = sd_nvic_EnableIRQ(ADC_IRQn);
    APP_ERROR_CHECK(err_code);

    NRF_ADC->EVENTS_END  = 0;    // Stop any running conversions.
    NRF_ADC->TASKS_START = 1;
}


/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t             err_code;
    ble_gap_adv_params_t adv_params;
    ble_gap_whitelist_t  whitelist;
    uint32_t             count;


    // Verify if there is any flash access pending, if yes delay starting advertising until
    // it's complete.
    err_code = pstorage_access_status_get(&count);
    APP_ERROR_CHECK(err_code);

    if (count != 0)
    {
        m_memory_access_in_progress = true;
        return;
    }

    // Initialize advertising parameters with defaults values
    memset(&adv_params, 0, sizeof(adv_params));
    
    adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    adv_params.p_peer_addr = NULL;
    adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    adv_params.p_whitelist = NULL;

    // Configure advertisement according to current advertising state
    switch (m_advertising_mode)
    {
        case BLE_NO_ADV:
            m_advertising_mode = BLE_FAST_ADV_WHITELIST;
            // fall through.

        case BLE_FAST_ADV_WHITELIST:
        {
            ble_gap_addr_t       * p_whitelist_addr[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            ble_gap_irk_t        * p_whitelist_irk[BLE_GAP_WHITELIST_IRK_MAX_COUNT];
            
            whitelist.addr_count = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
            whitelist.irk_count  = BLE_GAP_WHITELIST_IRK_MAX_COUNT;
            whitelist.pp_addrs   = p_whitelist_addr;
            whitelist.pp_irks    = p_whitelist_irk;

            err_code = dm_whitelist_create(&m_app_handle, &whitelist);
            APP_ERROR_CHECK(err_code);

            if ((whitelist.addr_count != 0) || (whitelist.irk_count != 0))
            {
                adv_params.fp          = BLE_GAP_ADV_FP_FILTER_CONNREQ;
                adv_params.p_whitelist = &whitelist;

                advertising_init(BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED);
                m_advertising_mode = BLE_FAST_ADV;
            }
            else
            {
                m_advertising_mode = BLE_SLOW_ADV;
            }

            adv_params.interval = APP_ADV_INTERVAL_FAST;
            adv_params.timeout  = APP_FAST_ADV_TIMEOUT;
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
            APP_ERROR_CHECK(err_code);
            break;
        }

        case BLE_FAST_ADV:
            advertising_init(BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE);

            adv_params.interval = APP_ADV_INTERVAL_FAST;
            adv_params.timeout  = APP_FAST_ADV_TIMEOUT;
            m_advertising_mode  = BLE_FAST_ADV;//BLE_SLOW_ADV;
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_SLOW_ADV:
            advertising_init(BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE);

            adv_params.interval = APP_ADV_INTERVAL_SLOW;
            adv_params.timeout  = APP_SLOW_ADV_TIMEOUT;
            m_advertising_mode  = BLE_SLOW_ADV;//BLE_SLEEP;
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_SLOW);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }

    // Start advertising.
    err_code = sd_ble_gap_adv_start(&adv_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *          This function will start the ADC.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    adc_start();
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    uint32_t err_code;

    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);

    // Create battery timer.
    err_code = app_timer_create(&m_battery_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

	
	
	//BLE_GAP_ADDR_TYPE_PUBLIC
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)local_dev_name.name,
                                          local_dev_name.len);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_KEYRING);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_tx_power_set(TX_POWER_LEVEL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 *
 * @param[in]  adv_flags  Indicates which type of advertisement to use, see @ref BLE_GAP_DISC_MODES.
 *
 */
static void advertising_init(uint8_t adv_flags)
{
#if 0
    uint32_t      err_code;
    ble_advdata_t advdata;
    int8_t        tx_power_level = TX_POWER_LEVEL;

//    ble_uuid_t adv_uuids[] =
//    {
//        {BLE_UUID_TX_POWER_SERVICE,        BLE_UUID_TYPE_BLE},
//        {BLE_UUID_IMMEDIATE_ALERT_SERVICE, BLE_UUID_TYPE_BLE},
//        {BLE_UUID_LINK_LOSS_SERVICE,       BLE_UUID_TYPE_BLE}
//    };
	ble_uuid_t adv_uuids[] =
    {
        {0x1111,        BLE_UUID_TYPE_BLE}
        {0X00FF, 		BLE_UUID_TYPE_BLE},
        {0XF00F,       	BLE_UUID_TYPE_BLE}
    };
	
    m_advertising_mode = BLE_FAST_ADV;//BLE_NO_ADV;

    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_NO_NAME;//BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags.size              = sizeof(adv_flags);
    advdata.flags.p_data            = &adv_flags;
    advdata.p_tx_power_level        = &tx_power_level;
    advdata.uuids_more_available.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    advdata.uuids_more_available.p_uuids  = adv_uuids;
    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);
	
#endif
	
	uint32_t        err_code;
    ble_advdata_t   advdata;
	
    ble_advdata_manuf_data_t manuf_specific_data;
    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;
    manuf_specific_data.data.p_data        = (uint8_t *) m_beacon_info;
    manuf_specific_data.data.size          = APP_BEACON_INFO_LENGTH;

    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type               = BLE_ADVDATA_NO_NAME;
    advdata.flags.size              = sizeof(adv_flags);
    advdata.flags.p_data            = &adv_flags;
    advdata.p_manuf_specific_data   = &manuf_specific_data;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);
	
}


/**@brief Function for initializing the services that will be used by the application.
 */
static void services_init(void)
{
#if 0
    tps_init();
    ias_init();
    lls_init();
    bas_init();
    ias_client_init();
#endif
#ifdef BLE_DFU_APP_SUPPORT    
    /** @snippet [DFU BLE Service initialization] */
    ble_dfu_init_t   dfus_init;

    // Initialize the Device Firmware Update Service.
    memset(&dfus_init, 0, sizeof(dfus_init));

    dfus_init.evt_handler    = dfu_app_on_dfu_evt;
    dfus_init.error_handler  = NULL; //service_error_handler - Not used as only the switch from app to DFU mode is required and not full dfu service.
    dfus_init.evt_handler    = dfu_app_on_dfu_evt;
    dfus_init.revision       = DFU_REVISION;

    uint32_t err_code = ble_dfu_init(&m_dfus, &dfus_init);
    APP_ERROR_CHECK(err_code);
    
    dfu_app_reset_prepare_set(reset_prepare);
    /** @snippet [DFU BLE Service initialization] */
#endif // BLE_DFU_APP_SUPPORT  
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = true;
    cp_init.evt_handler                    = NULL;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}




/**@brief Function for handling Immediate Alert events.
 *
 * @details This function will be called for all Immediate Alert events which are passed to the
 *          application.
 *
 * @param[in]   p_ias  Immediate Alert structure.
 * @param[in]   p_evt  Event received from the Immediate Alert service.
 */
//static void on_ias_evt(ble_ias_t * p_ias, ble_ias_evt_t * p_evt)
//{
//    switch (p_evt->evt_type)
//    {
//        case BLE_IAS_EVT_ALERT_LEVEL_UPDATED:
//            alert_signal(p_evt->params.alert_level);
//            break;

//        default:
//            // No implementation needed.
//            break;
//    }
//}


/**@brief Function for handling Link Loss events.
 *
 * @details This function will be called for all Link Loss events which are passed to the
 *          application.
 *
 * @param[in]   p_lls  Link Loss structure.
 * @param[in]   p_evt  Event received from the Link Loss service.
 */
//static void on_lls_evt(ble_lls_t * p_lls, ble_lls_evt_t * p_evt)
//{
////    switch (p_evt->evt_type)
////    {
////        case BLE_LLS_EVT_LINK_LOSS_ALERT:
////            alert_signal(p_evt->params.alert_level);
////            break;

////        default:
////            // No implementation needed.
////            break;
////    }
//}


/**@brief Function for handling IAS Client events.
 *
 * @details This function will be called for all IAS Client events which are passed to the
 *          application.
 *
 * @param[in]   p_ias_c  IAS Client structure.
 * @param[in]   p_evt    Event received.
 */
//static void on_ias_c_evt(ble_ias_c_t * p_ias_c, ble_ias_c_evt_t * p_evt)
//{
////    uint32_t err_code;
////    switch (p_evt->evt_type)
////    {
////        case BLE_IAS_C_EVT_SRV_DISCOVERED:
////            // IAS is found on peer. The Find Me Locator functionality of this app will work.
////            break;

////        case BLE_IAS_C_EVT_SRV_NOT_FOUND:
////            // IAS is not found on peer. The Find Me Locator functionality of this app will NOT work.
////            break;

////        case BLE_IAS_C_EVT_DISCONN_COMPLETE:
////            // Stop detecting button presses when not connected
////            err_code = bsp_buttons_enable(BSP_BUTTONS_NONE);
////            APP_ERROR_CHECK(err_code);
////            break;

////        default:
////            break;
////    }
//}


/**@brief Function for handling the Battery Service events.
 *
 * @details This function will be called for all Battery Service events which are passed to the
 |          application.
 *
 * @param[in]   p_bas  Battery Service structure.
 * @param[in]   p_evt  Event received from the Battery Service.
 */
//static void on_bas_evt(ble_bas_t * p_bas, ble_bas_evt_t * p_evt)
//{
////    uint32_t err_code;

////    switch (p_evt->evt_type)
////    {
////        case BLE_BAS_EVT_NOTIFICATION_ENABLED:
////            // Start battery timer
////            err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
////            APP_ERROR_CHECK(err_code);
////            break;

////        case BLE_BAS_EVT_NOTIFICATION_DISABLED:
////            err_code = app_timer_stop(m_battery_timer_id);
////            APP_ERROR_CHECK(err_code);
////            break;

////        default:
////            // No implementation needed.
////            break;
////    }
//}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t        err_code      = NRF_SUCCESS;
    

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            
            m_advertising_mode = BLE_NO_ADV;
            m_conn_handle      = p_ble_evt->evt.gap_evt.conn_handle;

            // Start handling button presses
            err_code = bsp_buttons_enable( (1 << SIGNAL_ALERT_BUTTON_ID) | (1 << STOP_ALERTING_BUTTON_ID) );
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);

            m_conn_handle = BLE_CONN_HANDLE_INVALID;

            //err_code = bsp_buttons_enable(BSP_BUTTONS_NONE);
            //APP_ERROR_CHECK(err_code);

            advertising_start();
            break;

        case BLE_GAP_EVT_TIMEOUT:
            if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISEMENT)
            {
//                if (m_advertising_mode == BLE_SLEEP)
//                {
//                    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
//                    APP_ERROR_CHECK(err_code);
//                    
//                    m_advertising_mode = BLE_NO_ADV;

//                    err_code = bsp_buttons_enable( (1 << WAKEUP_BUTTON_ID) | (1 << BOND_DELETE_ALL_BUTTON_ID));
//                    APP_ERROR_CHECK(err_code);

//                    // Go to system-off mode
//                    // (this function will not return; wakeup will cause a reset)
//                    err_code = sd_power_system_off();
//                    APP_ERROR_CHECK(err_code);
//                }
//                else
                {
                    advertising_start();
                }
            }
            break;

        case BLE_GATTC_EVT_TIMEOUT:
        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server and Client timeout events.
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for handling the Application's system events.
 *
 * @param[in]   sys_evt   system event.
 */
static void on_sys_evt(uint32_t sys_evt)
{
    switch(sys_evt)
    {
        case NRF_EVT_FLASH_OPERATION_SUCCESS:
        case NRF_EVT_FLASH_OPERATION_ERROR:

            if (m_memory_access_in_progress)
            {
                m_memory_access_in_progress = false;
                advertising_start();
            }
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**
* @brief RBC_MESH framework event handler. Defined in rbc_mesh.h. Handles
*   events coming from the mesh. Sets LEDs according to data
*
* @param[in] evt RBC event propagated from framework
*/
void rbc_mesh_event_handler(rbc_mesh_event_t* evt)
{
    TICK_PIN(28);
	static int i = 0;
	static int conf = 0;
	static int updata = 0;
	
    switch (evt->event_type)
    {
        case RBC_MESH_EVENT_TYPE_CONFLICTING_VAL:
			conf++;
			break;
        case RBC_MESH_EVENT_TYPE_NEW_VAL:
			updata++;
			//break;
        case RBC_MESH_EVENT_TYPE_UPDATE_VAL:
        
			i++;
//			if(evt->data[0] & 0x01)
//				pwm_evt_handler(255, 255, 255);
//			else
//				pwm_evt_handler(1, 1, 1);
            if (evt->value_handle == 1)
				dispose_recv_pkt(evt, evt->data, evt->data_len);

            //led_config(evt->value_handle, evt->data[0]);
            break;
		default :
			break;
    }
}



/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    dm_ble_evt_handler(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
//    ble_ias_on_ble_evt(&m_ias, p_ble_evt);
//    ble_lls_on_ble_evt(&m_lls, p_ble_evt);
//    ble_bas_on_ble_evt(&m_bas, p_ble_evt);
//    ble_ias_c_on_ble_evt(&m_ias_c, p_ble_evt);
    on_ble_evt(p_ble_evt);
	rbc_mesh_ble_evt_handler(p_ble_evt);
#ifdef BLE_DFU_APP_SUPPORT    
    /** @snippet [Propagating BLE Stack events to DFU Service] */
    ble_dfu_on_ble_evt(&m_dfus, p_ble_evt);
    /** @snippet [Propagating BLE Stack events to DFU Service] */
#endif // BLE_DFU_APP_SUPPORT
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    on_sys_evt(sys_evt);
	rbc_mesh_sys_evt_handler(sys_evt);
	
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
    //SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, false);
	SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_8000MS_CALIBRATION, false);	//for led pcb
	
	//SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_TEMP_8000MS_CALIBRATION, false);
    // Enable BLE stack 
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Device Manager events.
 *
 * @param[in]   p_evt   Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           api_result_t        event_result)
{
    APP_ERROR_CHECK(event_result);
	switch(p_event->event_id)
    {
#ifdef BLE_DFU_APP_SUPPORT    
        case DM_EVT_DEVICE_CONTEXT_LOADED: // Fall through.
        case DM_EVT_SECURITY_SETUP_COMPLETE:
            dfu_app_set_dm_handle(p_handle);
            break;
        case DM_EVT_DISCONNECTION:
            dfu_app_set_dm_handle(NULL);
            break;
#endif // BLE_DFU_APP_SUPPORT    
    }
    return NRF_SUCCESS;
}


/**@brief Function for the Device Manager initialization.
 */
static void device_manager_init(void)
{
    uint32_t               err_code;
    dm_init_param_t        init_data;
    dm_application_param_t register_param;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    // Clear all bonded centrals if the Bonds Delete button is pushed.
    err_code = bsp_button_is_pressed(BOND_DELETE_ALL_BUTTON_ID, &(init_data.clear_persistent_data));
    APP_ERROR_CHECK(err_code);
    
    err_code = dm_init(&init_data);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));
    
    register_param.sec_param.timeout      = SEC_PARAM_TIMEOUT;
    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
    // Initialize.
    app_trace_init();
    timers_init();
//    gpiote_init();
//    err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), button_event_handler);
//    APP_ERROR_CHECK(err_code);
	//{  add for seelight 
    pwm_init();
	start_timer2();
	start_timer1(); 
	//}					
	ble_stack_init();
	//{
	dispose_pkt_init();
	//}
    device_manager_init();
    gap_params_init();
    advertising_init(BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE);
    services_init();
    conn_params_init();
	//{
	//dispose_pkt_init();
	//}
	/*init for ble mesh*/
	rbc_mesh_init_params_t init_params;
	init_params.access_addr = 0xA541A68F;
    init_params.adv_int_ms = 100;
    init_params.channel = 38;
    init_params.handle_count = 2;                                //the number of character by user
    init_params.packet_format = RBC_MESH_PACKET_FORMAT_ORIGINAL;
    init_params.radio_mode = RBC_MESH_RADIO_MODE_BLE_1MBIT;
    
    err_code = rbc_mesh_init(init_params);
    APP_ERROR_CHECK(err_code);
	/*end of init for ble mesh*/
	
	
    // Start execution.
    advertising_start();

    // Enter main loop.
    for (;; )
    {
        power_manage();
    }
}


/**
 * @}
 */
