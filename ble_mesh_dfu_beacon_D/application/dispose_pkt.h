#ifndef __DISPOSE_PKT_H_

#define __DISPOSE_PKT_H_
#include "stdint.h"
typedef struct BDADDR
{
	uint8_t  dst_addr[6];
	uint8_t  src_addr[6];
}bdaddr_t;
 
typedef struct OP
{
	uint8_t T:1;
	uint8_t opcode:7;
}opcode_t;

typedef struct 
{
	uint8_t B:1;
	uint8_t E:1;
	uint8_t len:6;
}len_t;

typedef struct 
{
	opcode_t opcode;
	len_t    len;
	uint8_t  sequnce;
	uint8_t  check_sum;
	uint8_t  payload[1];
}trans_data_t;

enum
{
	config_color        = 0x00,
	config_brightness,
	config_name,	
	get_devices_list,
	get_device_name,
//add scenery mode
	power_on,
	power_off,
	mode_flow,
	mode_candles,
	mode_strobe,
	
	mode_flash,
	mode_smooth,
	mode_listen
//end
};

typedef struct
{
	uint16_t  len;
	uint8_t   name[31];
}dev_name_t;
void recv_pkt_from_mesh(uint8_t *data, uint8_t len); 
void dispose_recv_pkt(uint8_t *data, uint8_t len);
void dispose_pkt_init(void);
void generate_pkt_2_mesh(uint8_t *mesh_data, uint8_t type);
#endif 

