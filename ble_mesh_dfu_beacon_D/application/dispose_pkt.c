#include "dispose_pkt.h"
#include "ble_gap.h"
#include <string.h>
#include "rbc_mesh.h"
#include "pwm.h"
#include "app_timer.h"
#include "switch_color.h"
#define HEADER_LEN			10//16
static uint8_t version = 0;
static ble_gap_addr_t local_addr;
dev_name_t     local_dev_name;


uint8_t broadcast_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

bdaddr_t recv_pkt_addr_hdr;

extern int r_value;            
extern int g_value;  
extern int b_value;
//static uint8_t brightness = 100;
static struct 
{
	uint16_t total_len; 		//包括包头的总长度
	uint8_t  data[1024];
}recv_data;



#if 1   // add scenery mode
#define NUMBER_OF_COLOR                8
#define RFSTAR_APP_TIMER_PRESCALER     0
#define LIGHT_OFF                      0
#define LIGHT_ON                       1
static app_timer_id_t                   m_auto_chg_timer_id;
static uint8_t                          chg_mode      = 0;
static uint8_t                          color_pointer = 0;
static uint8_t                          r_values[8]  = { 0xfe, 0x01, 0xff - 0x10, 0x01, 0xff - 0xc9, 0xfe, 0x01, 0xfe};
static uint8_t                          g_values[8]  = { 0x01, 0xfe, 0xff - 0xa3, 0x01, 0xff - 0x16, 0xfe, 0xfe, 0x01};
static uint8_t                          b_values[8]  = { 0x01, 0x01, 0xff - 0x90, 0xfe, 0xff - 0x6f, 0x01, 0xfe, 0xfe};
static uint8_t                          current_light_state = 0;
static uint8_t                          current_light_value = 0;
static bool								is_increase         = true;
static void auto_chg_mode_handler(void *p_context)
{
	static COLOR_RGB rgb = {0};
    static COLOR_HSL hsl = {0};
	switch(chg_mode)
    {
        case mode_flow:
        {
			pwm_evt_handler(r_values[color_pointer], g_values[color_pointer], b_values[color_pointer]);
            color_pointer = (color_pointer + 1) % NUMBER_OF_COLOR;
            app_timer_start(m_auto_chg_timer_id, 
                APP_TIMER_TICKS(700, RFSTAR_APP_TIMER_PRESCALER), 
                NULL);
            
        }
            break;

        case mode_candles:
        {
            if(current_light_state == LIGHT_ON)
            {
                pwm_evt_handler(0xff, 0xff, 0xff);
                current_light_state = LIGHT_OFF;
                app_timer_start(m_auto_chg_timer_id, 
                                APP_TIMER_TICKS(200 , RFSTAR_APP_TIMER_PRESCALER), 
                                NULL);
                return;
            }
            if(current_light_state == LIGHT_OFF)
            {
                pwm_evt_handler(r_values[color_pointer], g_values[color_pointer], b_values[color_pointer]);
                current_light_state = LIGHT_ON;
                color_pointer = (color_pointer + 1) % NUMBER_OF_COLOR;
                app_timer_start(m_auto_chg_timer_id, 
                                APP_TIMER_TICKS(1000, RFSTAR_APP_TIMER_PRESCALER), 
                                NULL);
            }
            
        }
            break;
        case mode_strobe:
        {
            
            if(current_light_value == 1)
            {
                rgb.red   = r_values[color_pointer];
                rgb.green = g_values[color_pointer];
                rgb.blue  = b_values[color_pointer];
                rgb_to_hsl(&rgb, &hsl);
                color_pointer++;
                
            }
            current_light_value = (uint8_t)hsl.luminance;
            hsl.luminance = --current_light_value;
            hsl_to_rgb(&hsl, &rgb);
			pwm_evt_handler(255-rgb.red, 255-rgb.green, 255-rgb.blue);
            app_timer_start(m_auto_chg_timer_id, 
                            APP_TIMER_TICKS(500 , RFSTAR_APP_TIMER_PRESCALER), 
                            NULL);
        }

            break;

        case mode_flash:
		{
		
			if(current_light_state == LIGHT_ON)
            {
                pwm_evt_handler(0xff, 0xff, 0xff);
                current_light_state = LIGHT_OFF;
                app_timer_start(m_auto_chg_timer_id, 
                                APP_TIMER_TICKS(200 , RFSTAR_APP_TIMER_PRESCALER), 
                                NULL);
                return;
            }
            if(current_light_state == LIGHT_OFF)
            {
                pwm_evt_handler(r_values[color_pointer], g_values[color_pointer], b_values[color_pointer]);
                current_light_state = LIGHT_ON;
                color_pointer = (color_pointer + 1) % NUMBER_OF_COLOR;
                app_timer_start(m_auto_chg_timer_id, 
                                APP_TIMER_TICKS(700, RFSTAR_APP_TIMER_PRESCALER), 
                                NULL);
            }
		}
			break;
		
		case mode_smooth:
		{
		
			if(current_light_value <= 5 || current_light_value >= 80)
            {
                rgb.red   = r_values[color_pointer];
                rgb.green = g_values[color_pointer];
                rgb.blue  = b_values[color_pointer];
                rgb_to_hsl(&rgb, &hsl);
                color_pointer++;
                if(current_light_value <= 5)
				{
					is_increase = true;
					hsl.luminance = 5;
				}
				else
				{
					is_increase = false;
					hsl.luminance = 80;
				}
            }
            current_light_value = (uint8_t)hsl.luminance;
			if(is_increase == false)
				hsl.luminance = --current_light_value;
			else
				hsl.luminance = ++current_light_value;
            hsl_to_rgb(&hsl, &rgb);
			pwm_evt_handler(255-rgb.red, 255-rgb.green, 255-rgb.blue);
            app_timer_start(m_auto_chg_timer_id, 
                            APP_TIMER_TICKS(500 , RFSTAR_APP_TIMER_PRESCALER), 
                            NULL);
		}
		
			break;
		
		
		case mode_listen:
		{
			pwm_evt_handler(r_values[color_pointer], g_values[color_pointer], b_values[color_pointer]);
            color_pointer = (color_pointer + 1) % NUMBER_OF_COLOR;
            app_timer_start(m_auto_chg_timer_id, 
                APP_TIMER_TICKS(500, RFSTAR_APP_TIMER_PRESCALER), 
                NULL);
			
		}
		
			break;

        default:

            break;
    }
}
static void sl_app_timer_init()
{
	app_timer_create(&m_auto_chg_timer_id, 
                                APP_TIMER_MODE_SINGLE_SHOT,
                                auto_chg_mode_handler);
}


#endif // end scenery mode
static void bdaddr_2_str(uint8_t *src, char *dst, int len)
{
	int i = 0;
	int j = 0;
	int tmp = 0;
	for(i=0; i < len; i++)
	{
		tmp = src[i] & 0x0f;
		if(tmp <=9 && tmp >=0)
			dst[j++] = tmp + '0';
		else
			dst[j++] = tmp - 10 + 'a';
		tmp = (src[i] >> 4) & 0x0f;
		if(tmp <=9 && tmp >=0)
			dst[j++] = tmp + '0';
		else
			dst[j++] = tmp - 10 + 'a';

	}
}

void dispose_pkt_init()
{
	uint32_t err_code;
	char dev_name[20] = "led_";
	ble_gap_conn_sec_mode_t sec_mode;
	//add scenery mode
	sl_app_timer_init();
	//
	sd_ble_gap_address_get(&local_addr);
	bdaddr_2_str(local_addr.addr, &dev_name[4], 2);
	local_dev_name.len = 31;
	err_code = sd_ble_gap_device_name_get(local_dev_name.name, &local_dev_name.len);
	memcpy(local_dev_name.name, dev_name, strlen(dev_name)); 
	local_dev_name.len = strlen(dev_name);
//	err_code = sd_ble_gap_device_name_set(&sec_mode,
//                                          (const uint8_t *)dev_name,
//                                          strlen(dev_name));
    //APP_ERROR_CHECK(err_code);
	memset(&recv_data, 0, sizeof(recv_data));
}


void recv_pkt_from_mesh(uint8_t *data, uint8_t len)
{
	uint8_t index = 0;
//	static uint8_t mesh_data[28] = { 0 };
	if(data[index] == version)
		return ;
	version = data[index];
	index = index + sizeof(uint8_t);
	bdaddr_t *addr = (bdaddr_t *)&data[index];
	memcpy(&recv_pkt_addr_hdr, addr, sizeof(bdaddr_t));
	if(!memcmp(addr->dst_addr, broadcast_addr, 6) || !memcmp(addr->dst_addr, local_addr.addr, 6))
	{
		//index = index + sizeof(bdaddr_t);
		index = index + 6;
		trans_data_t *trans = (trans_data_t *)&data[index];
		if(trans->len.B && trans->len.E)// one packet
		{
			dispose_recv_pkt(data, len);
		}
		else if(trans->len.B)           // 
		{
			memset(&recv_data, 0, sizeof(recv_data));
			memcpy(recv_data.data, data, HEADER_LEN);
			recv_data.total_len = recv_data.total_len + HEADER_LEN;
		}
		else if(trans->len.E)
		{
			memcpy(&recv_data.data[recv_data.total_len], &data[HEADER_LEN], trans->len.len);
			recv_data.total_len = recv_data.total_len + trans->len.len;
			recv_data.data[0] = version;
			dispose_recv_pkt(recv_data.data, recv_data.total_len);
		}
	}
}

void dispose_recv_pkt(uint8_t *data, uint8_t len)
{
	uint8_t index = 0;
	static uint8_t mesh_data[28] = { 0 };
//	if(data[index] == version)
//		return ;
//	version = data[index];
	if(len < HEADER_LEN)
		return ;
//	index = index + sizeof(uint8_t);
	bdaddr_t *addr = (bdaddr_t *)&data[index];
	memcpy(&recv_pkt_addr_hdr, addr, sizeof(bdaddr_t));
	if(!memcmp(addr->dst_addr, broadcast_addr, 6) || !memcmp(addr->dst_addr, local_addr.addr, 6))
	{
		//index = index + sizeof(bdaddr_t);
		index = index + 6;
		trans_data_t *trans = (trans_data_t *)&data[index];
		uint8_t opcode = trans->opcode.opcode;//data[index] >> 1;
		switch(opcode)
		{
		case config_color:
			app_timer_stop(m_auto_chg_timer_id);
			if(trans->len.len < 3)
				return ;
			if(r_value == trans->payload[0] && g_value == trans->payload[1] && b_value == trans->payload[2])
				return ;
			r_value = trans->payload[0];//255 - trans->payload[0];//[index];
			g_value = trans->payload[1];//255 - trans->payload[1];//data[index + 1];
			b_value = trans->payload[2];//255 - trans->payload[2];//data[index + 2];
			pwm_evt_handler(r_value, g_value, b_value);
			break;
		case config_brightness:
			
			break;
		
		case config_name:
			if(trans->len.len < 1)
				return ;
			memset(&local_dev_name, 0, sizeof(local_dev_name));
			local_dev_name.len = 31;
			memcpy(local_dev_name.name, trans->payload, trans->len.len);
			local_dev_name.len = trans->len.len;
			ble_gap_conn_sec_mode_t sec_mode;
			BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
			sd_ble_gap_device_name_set(&sec_mode, local_dev_name.name, local_dev_name.len);
			break;
		case get_devices_list:
			memset(mesh_data, 0, sizeof(mesh_data));
			generate_pkt_2_mesh(mesh_data, get_devices_list);
			rbc_mesh_value_set(2, &mesh_data[6], 20);
			break;
		
		case get_device_name:
			memset(mesh_data, 0, sizeof(mesh_data));
			generate_pkt_2_mesh(mesh_data, get_device_name);
			rbc_mesh_value_set(2, &mesh_data[6], 20);
			break;
		case power_on:
			app_timer_stop(m_auto_chg_timer_id);
			pwm_evt_handler(r_value, g_value, b_value);
			break;
		
		case power_off:
			app_timer_stop(m_auto_chg_timer_id);
			pwm_evt_handler(0xff, 0xff, 0xff);
			break;
		case mode_flow:
		case mode_candles:
		case mode_strobe:
			app_timer_stop(m_auto_chg_timer_id);
			color_pointer = 0;
			chg_mode      = opcode;
			current_light_state = LIGHT_OFF;
			current_light_value = 1;
			app_timer_start(m_auto_chg_timer_id, 
					APP_TIMER_TICKS(200, RFSTAR_APP_TIMER_PRESCALER), 
					NULL);
			break;
		
		case mode_flash:
		case mode_smooth:
		case mode_listen:
			app_timer_stop(m_auto_chg_timer_id);
			color_pointer = 6;
			chg_mode      = opcode;
			current_light_state = LIGHT_OFF;
			current_light_value = 1;
			app_timer_start(m_auto_chg_timer_id, 
					APP_TIMER_TICKS(200, RFSTAR_APP_TIMER_PRESCALER), 
					NULL);
			break;
		default:
			
			break;
		}
	}
}

void generate_pkt_2_mesh(uint8_t *mesh_data, uint8_t type)
{
	uint8_t index = 0;
	switch(type)
		{
		case get_devices_list:
//			mesh_data[index] = version + 1; 
//			index = index + 1;
		{
			bdaddr_t *addr = (bdaddr_t *)&mesh_data[index]; 
			memcpy(addr->dst_addr, recv_pkt_addr_hdr.src_addr, 6);
			memcpy(addr->src_addr, local_addr.addr, 6);
			index = index + sizeof(bdaddr_t);
			mesh_data[index++] = get_devices_list << 1;
			mesh_data[index++] = 7;
			mesh_data[index++] = 0;
			memcpy(&mesh_data[index], local_addr.addr, 6);
		}
			break;
		
		case get_device_name:
			
//			mesh_data[index] = version + 1; 
//			index = index + 1;
		{
			bdaddr_t *addr1 = (bdaddr_t *)&mesh_data[index];
			memcpy(addr1->dst_addr, recv_pkt_addr_hdr.src_addr, 6);
			memcpy(addr1->src_addr, local_addr.addr, 6);
			index = index + sizeof(bdaddr_t);
			trans_data_t *trans = (trans_data_t *)&mesh_data[index];
			trans->opcode.opcode = get_device_name;
			trans->len.len = local_dev_name.len;
			trans->sequnce = 0;
			trans->check_sum = 0;
			memcpy(trans->payload, local_dev_name.name, local_dev_name.len);
			//mesh_data[index++] = 0;
			//memcpy(&mesh_data[index], local_addr.addr, 6);
		}
			break;
		
		default:
			
			break;
		}
}

