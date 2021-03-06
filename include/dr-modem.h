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



#ifndef _DR_DPRAM_H_
#define _DR_DPRAM_H_

#include "dr-main.h"


/** 
  *  @internal
  *  This function is used to control modem interface
  * 
  *   @param       [in]   status              DTR status
  *   @return       This function returns whether sending signal to modem via tapi to control modem interface is successful or not 
  */
int _send_dtr_ctrl_signal(int status);

#endif				//_DR_DPRAM_H_
