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

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <libARSAL/ARSAL.h>
#include <libARSAL/ARSAL_Print.h>
#include <libARNetwork/ARNetwork.h>
#include <libARNetworkAL/ARNetworkAL.h>
#include <libARCommands/ARCommands.h>
#include <libARDiscovery/ARDiscovery.h>
#include <libARStream/ARStream.h>

#ifdef __cplusplus 
}
#endif

#include "devicecontroller.h"

struct DEVICE_CONTROLLER_DATA {
  ARNETWORKAL_Manager_t* _alManager;
  ARNETWORK_Manager_t* _netManager;
  ARSTREAM_Reader_t* _streamReader;
  ARSAL_Thread_t _rxThread;
  ARSAL_Thread_t _txThread;
  ARSAL_Thread_t _videoTxThread;
  ARSAL_Thread_t _videoRxThread;
  int _d2cPort;
  int _c2dPort;
  int _arstreamFragSize;
  int _arstreamFragNb;
  int _arstreamAckDelay;
  uint8_t* _videoFrame;
  uint32_t _videoFrameSize;

  // FILE *video_out;
  // int _writeImgs;
  // int _frameNb;
};


DeviceController::DeviceController()
{

}

DeviceController::~DeviceController()
{

}

bool DeviceController::connect() {
  return false;
}

bool DeviceController::disconnect() {
  return false;
}

bool DeviceController::isConnected() {
  return false;
}
  
RACEDRONE_VALUES_T DeviceController::getValues() {
  RACEDRONE_VALUES_T pv;

  pv.alert = rand() % 2;
  pv.batteryChargePercentage = rand() % 100;
  pv.connected = true;
  pv.posture = "normal";
  pv.speedInCmPerSec = rand() % 30;
  pv.speedVal = rand() % 100;
  pv.turnVal = rand() % 100;
  pv.linkQuality = rand() % 7;

  return pv;
}

