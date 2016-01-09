/*
 * Copyright 2016 <copyright holder> <email>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#ifndef DEVICECONTROLLER_H
#define DEVICECONTROLLER_H

#include "model/racedronedata.h"

struct DEVICE_CONTROLLER_DATA;
typedef struct DEVICE_CONTROLLER_DATA DEVICE_CONTROLLER_DATA_T;

class DeviceController
{
public:
  DeviceController();
  ~DeviceController();
  
  bool connect();
  bool disconnect();
  bool isConnected();
  
  RACEDRONE_VALUES_T getValues();
  
private:
  DEVICE_CONTROLLER_DATA_T* _handle;
};

#endif // DEVICECONTROLLER_H