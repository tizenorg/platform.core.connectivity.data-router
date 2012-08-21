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


#include <ITapiPS.h>
#include <tapi_common.h>
#include <ITapiModem.h>
#include "dr-modem.h"
#include "dr-usb.h"
#include "dr-common.h"



extern dr_info_t dr_info;
extern TapiHandle *tapi_handle;



static void __modem_pin_ctrl_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
	DBG("__modem_pin_ctrl_cb result : %d \n", result);
	return;
}

int _send_dtr_ctrl_signal(int status)
{
	int result;
	tapi_ps_btdun_pincontrol pincontrol;

	pincontrol.signal = GPRS_DATA_SIGNAL_DTR;
	if (status == DTR_ON) {
		pincontrol.status = GPRS_SIGNAL_STATUS_ON;
		DBG("Send DTR+ to Modem \n");
	} else {
		pincontrol.status = GPRS_SIGNAL_STATUS_OFF;
		DBG("Send DTR- to Modem \n");
	}

	result = tel_control_modem_dun_pin_ctrl(tapi_handle, &pincontrol, __modem_pin_ctrl_cb, NULL);
	if (result != TAPI_API_SUCCESS) {
		ERR("DTR sending failed. [Error : %d]\n", result);
	}

	return result;
}
