#ifndef __FLAH_OPT_H__
#define __FLAH_OPT_H__
#include "pstorage.h"
void usr_clear_flash(void);
void usr_falsh_init(void);
void store_device_name(char *name, uint8_t len);
void load_device_name(char *name, uint8_t len);
#endif

