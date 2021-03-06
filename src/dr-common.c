/*
 * Data-Router
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact:  Hocheol Seo <hocheol.seo@samsung.com>
 *               Injun Yang <injun.yang@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include <glib.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "dr-main.h"
#include "dr-common.h"
#include "dr-util.h"

int _get_usb_state(int *usb_state)
{
	if (-1 == vconf_get_int(VCONFKEY_SYSMAN_USB_STATUS, (void *)usb_state)) {
		ERR("Vconf get failed\n");
		return -1;
	}
	DBG("USB state : %d\n", *usb_state);
	return 0;
}

