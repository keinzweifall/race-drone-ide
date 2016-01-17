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

#include <boost/thread.hpp>

#include "model/racedronedata.h"
#include "model/videoframefeed.h"

struct DEVICE_CONTROLLER_DATA;
typedef struct DEVICE_CONTROLLER_DATA DEVICE_CONTROLLER_DATA_T;
class CmdQueue;

class DeviceController
{
public:
  DeviceController();
  ~DeviceController();
  
  bool connect();
  bool disconnect();
  bool isConnected();
  bool enterControlLoop();
  bool exitControlLoop();
  int ping();
  RACEDRONE_VALUES_T getValues();

  VideoFrameFeed* startVideo();
  void stopVideo();
  
  void setCmdQueue(CmdQueue* q) { _cmdq = q; }

private:
  DEVICE_CONTROLLER_DATA_T* _handle;
  CmdQueue* _cmdq;
  
  void _init_handle();
  bool _internal_connect();
  int _send_media_stream(bool streamOn);
  
  static void* _reader_loop(void* data);
  static void* _sender_loop(void* data);
};

#endif // DEVICECONTROLLER_H
