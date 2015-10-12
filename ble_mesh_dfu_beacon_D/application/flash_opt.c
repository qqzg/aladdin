#include <stdio.h>
#include "flash_opt.h"
#include "app_error.h"
#include "pstorage_platform.h"

static void pstorage_ntf_cb(pstorage_handle_t *  p_handle,
                                  uint8_t              op_code,
                                  uint32_t             result,
                                  uint8_t *            p_data,
                                  uint32_t             data_len)
{
	
}

static	pstorage_module_param_t p_module_param = {
	pstorage_ntf_cb,
	64,
	1
	};
#define DEVICENAME_BLOCK	0
static	pstorage_handle_t base_block_id;
	
	
void usr_falsh_init(void)
{
	uint32_t err_code;
	//err_code = PSTORAGE_SWAP_ADDR;
	err_code = pstorage_register(&p_module_param,	&base_block_id); 
	APP_ERROR_CHECK(err_code);
}

void usr_clear_flash(void)
{
	uint32_t err_code;
	err_code = pstorage_clear(&base_block_id, 64*1);
	APP_ERROR_CHECK(err_code);
}

void store_device_name(char *name, uint8_t len)
{
	uint32_t err_code; 
	pstorage_handle_t p_block_id;
	if(len > 36)
		return ;
	err_code = pstorage_block_identifier_get(&base_block_id, DEVICENAME_BLOCK, &p_block_id);
	APP_ERROR_CHECK(err_code);
	err_code = pstorage_store(&p_block_id, (uint8_t *)name, len, 0);
	APP_ERROR_CHECK(err_code);
}


void load_device_name(char *name, uint8_t len)
{
	uint32_t err_code;
	pstorage_handle_t p_block_id;
	err_code = pstorage_block_identifier_get(&base_block_id, DEVICENAME_BLOCK, &p_block_id);
	APP_ERROR_CHECK(err_code);
	err_code = pstorage_load((uint8_t *)name, &p_block_id, len, 0);
	APP_ERROR_CHECK(err_code);
}


