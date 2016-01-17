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

#ifndef CMDQUEUE_H
#define CMDQUEUE_H

#include <queue>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef __cplusplus
extern "C" {
#endif

#include <libARCommands/ARCommands.h>
  
#ifdef __cplusplus 
}
#endif



typedef enum {
  eCMD_NOCOMMAND = 0,
  eCMD_EMERGENCYSTOP,
  eCMD_PILOTING,
  eCMD_ANIMATION
} eCMD_TYPE;

typedef struct {
  eCMD_TYPE type;

  // when was command issued -> important for TTL
  boost::posix_time::ptime issued;
  
  // piloting parameters
  uint8_t flag;
  int8_t speed;
  int8_t turn;

  // animation parameters
  eARCOMMANDS_JUMPINGSUMODEBUG_ANIMATION_PLAYANIMATION_ID animation;
} COMMAND_T;

COMMAND_T create_NO_COMMAND();
COMMAND_T create_EMERGENCY_STOP();
COMMAND_T create_PILOTING(uint8_t flag, int8_t speed, int8_t turn);
COMMAND_T create_ANIMATION(eARCOMMANDS_JUMPINGSUMODEBUG_ANIMATION_PLAYANIMATION_ID);

class CmdQueue
{
public:
  CmdQueue(unsigned long pMaxCmds, unsigned long TTL = 0);
  ~CmdQueue();
  
  void put(COMMAND_T cmd);
  COMMAND_T get();
  
  void emergencyStop();
  void clear();
  
private:
  std::queue<COMMAND_T> _cmds;
  unsigned long _maxCmds;
  unsigned long _TTL;
  boost::mutex _mtx;
};

#endif // CMDQUEUE_H
