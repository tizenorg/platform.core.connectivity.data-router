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
#include <sys/wait.h>
#include <ITapiPS.h>
#include <tapi_common.h>
#include <ITapiModem.h>

#include "dr-main.h"
#include "dr-usb.h"
#include "dr-modem.h"
#include "dr-common.h"



#define ACM_WAIT_TIME 300000

extern dr_info_t dr_info;
volatile gboolean dsr_status = FALSE;
TapiHandle *tapi_handle = NULL;


static void __usb_noti_handler(void *data)
{
	int usb_state = -1;
	int usb_mode = -1;
	int ret;

	if (_get_usb_state(&usb_state) < 0) {
		ERR(" Unable to get usb status !\n");
		return;
	}

	ret = vconf_get_int(VCONFKEY_SETAPPL_USB_MODE_INT, &usb_mode);
	if (ret != 0) {
		DBG("Vconf get failed\n");
		return;
	}

	DBG("USB noti handler, USB state : %d, Mode : %d\n", usb_state, usb_mode);
	DBG("usb_fd = 0x%x\n", dr_info.usb.usb_fd);


	/*
	 * If USB driver builted in the Kernel, VCONFKEY_SYSMAN_USB_CONNECTED is not used.
	 * If USB driver loaded in DR/MTP then need to check VCONFKEY_SYSMAN_USB_CONNECTED.
	 * After usb driver loaded, MTP will be set vconf value as VCONFKEY_SYSMAN_USB_CONNECTED
	 */
	if (usb_state != VCONFKEY_SYSMAN_USB_DISCONNECTED &&
		usb_mode == SETTING_USB_SAMSUNG_KIES) {
		_init_usb();
	} else if (usb_state == VCONFKEY_SYSMAN_USB_DISCONNECTED) {
		_deinit_usb();
		_deinit_dr();
	}

	return;
}

static void __usb_mode_noti_handler(void *data)
{
	int usb_state = -1;
	int usb_mode = -1;
	int ret;

	ret = vconf_get_int(VCONFKEY_SETAPPL_USB_MODE_INT, &usb_mode);
	if (ret != 0) {
		DBG("Vconf get failed\n");
		return;
	}

	DBG("USB Mode noti handler, USB  Mode : %d\n", usb_mode);
	DBG("usb_fd = 0x%x\n", dr_info.usb.usb_fd);

	if (usb_mode == SETTING_USB_SAMSUNG_KIES) {
		if (_get_usb_state(&usb_state) < 0) {
			ERR(" Unable to get usb status !\n");
			return;
		}
		if (usb_state != VCONFKEY_SYSMAN_USB_DISCONNECTED) {
			usleep(ACM_WAIT_TIME);
			_init_usb();
		}
	} else if (usb_mode == SETTING_USB_MASS_STORAGE ||
			usb_mode == SETTING_USB_DEBUG_MODE ||
			usb_mode == SETTING_USB_MOBILE_HOTSPOT) {
		_deinit_usb();
		_deinit_dr();
	}

	return;
}


gboolean _register_vconf_notification(void)
{
	int ret;

	ret =
	    vconf_notify_key_changed(VCONFKEY_SYSMAN_USB_STATUS, (vconf_callback_fn) __usb_noti_handler, NULL);
	if (ret < 0) {
		ERR("Error !!! VCONFKEY reg noti  : %s\n", VCONFKEY_SYSMAN_USB_STATUS);
	}

	ret =
	    vconf_notify_key_changed(VCONFKEY_SETAPPL_USB_MODE_INT,
				     (vconf_callback_fn) __usb_mode_noti_handler, NULL);
	if (ret < 0) {
		ERR("Error !!! VCONFKEY reg noti  : %s\n", VCONFKEY_SETAPPL_USB_MODE_INT);
	}

	return TRUE;
}

void _unregister_vconf_notification(void)
{
	vconf_ignore_key_changed(VCONFKEY_SYSMAN_USB_STATUS, (vconf_callback_fn) __usb_noti_handler);
	vconf_ignore_key_changed(VCONFKEY_SETAPPL_USB_MODE_INT, (vconf_callback_fn) __usb_mode_noti_handler);
	return;
}


static void __dcd_pin_handler(tapi_ps_btdun_pincontrol_status status)
{
	DBG(" \n");

	if (status == GPRS_SIGNAL_STATUS_ON) {
		DBG("Receive DCD + from Modem\n");
		dr_info.line.output_line_state.bits.dcd = TRUE;
		DBG("Current modem output line status = 0x%X\n",
			     dr_info.line.output_line_state.state);
	}else if (status == GPRS_SIGNAL_STATUS_OFF) {
		DBG("Receive DCD - from Modem\n");
		dr_info.line.output_line_state.bits.dcd = FALSE;
		DBG("Current modem output line status = 0x%X\n",
			     dr_info.line.output_line_state.state);
	}
	_send_usb_line_state(dr_info.line.output_line_state.state);
}

static void __dsr_pin_handler(tapi_ps_btdun_pincontrol_status status)
{
	if (status == GPRS_SIGNAL_STATUS_ON) {
		dr_info.line.output_line_state.bits.dsr = TRUE;
		dsr_status = TRUE;
		DBG("Receive DSR+ from modem\n");
	}else if (status == GPRS_SIGNAL_STATUS_OFF) {
		dr_info.line.output_line_state.bits.dsr = FALSE;
		dsr_status = FALSE;

		DBG("Receive DSR- from modem\n");
	}
	_send_usb_line_state(dr_info.line.output_line_state.state);
}



static void __tel_dun_pincontrol_handler(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	DBG("+\n");

	if (data == NULL)
		return;

	tapi_ps_btdun_pincontrol *pinctrl;
	pinctrl = (tapi_ps_btdun_pincontrol *)data;

	DBG("pincontrol - Signal: %d, status: %d\n", pinctrl->signal, pinctrl->status);
	switch (pinctrl->signal) {
	case GPRS_DATA_SIGNAL_DSR:
		__dsr_pin_handler(pinctrl->status);
		break;
	case GPRS_SIGNAL_DCD:
		__dcd_pin_handler(pinctrl->status);
		break;
	default:
		break;
	}

	DBG("-\n");
	return;
}

gboolean _register_telephony_event(void)
{
	int ret;

	tapi_handle = tel_init(NULL);
	if (tapi_handle == NULL) {
		ERR("tel_init failed !!!\n");
		return FALSE;
	}

	ret = tel_register_noti_event(tapi_handle, TAPI_NOTI_MODEM_DUN_PIN_CTRL,
				 __tel_dun_pincontrol_handler,
				 NULL);

	return TRUE;
}

void _unregister_telephony_event(void)
{
	tel_deregister_noti_event(tapi_handle, TAPI_NOTI_MODEM_DUN_PIN_CTRL);

	if (tel_deinit(tapi_handle) != TAPI_API_SUCCESS) {
		ERR("tel_deinit failed !!!\n");
	}

	tapi_handle = NULL;

	return;
}

