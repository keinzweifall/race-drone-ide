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

#include <boost/log/trivial.hpp>

#include "cmdqueue.h"

COMMAND_T create_NO_COMMAND() {
  COMMAND_T c;
  c.type = eCMD_NOCOMMAND;
  return c;
}

COMMAND_T create_EMERGENCY_STOP() {
  COMMAND_T c;
  c.type = eCMD_EMERGENCYSTOP;
  return c;
}

COMMAND_T create_PILOTING(uint8_t flag, int8_t speed, int8_t turn) {
  COMMAND_T c;
  c.type = eCMD_PILOTING;
  c.flag = flag;
  c.speed = speed;
  c.turn = turn;
  return c;
}

COMMAND_T create_ANIMATION(eARCOMMANDS_JUMPINGSUMODEBUG_ANIMATION_PLAYANIMATION_ID id) {
  COMMAND_T c;
  c.type = eCMD_ANIMATION;
  c.animation = id;
  return c;
}


CmdQueue::CmdQueue(unsigned long pMaxCmds, unsigned long TTL) : _TTL(TTL), _maxCmds(pMaxCmds) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "CmdQueue::CmdQueue(pMaxCmds: " << pMaxCmds << ", TTL: " << TTL << ")";
}

CmdQueue::~CmdQueue() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "CmdQueue::~CmdQueue()";

  boost::lock_guard<boost::mutex> guard(_mtx);
  std::queue<COMMAND_T> empty;
  std::swap(_cmds, empty);
}

void CmdQueue::put(COMMAND_T cmd) {
  // BOOST_LOG_TRIVIAL(trace) << __LINE__ << "CmdQueue::setNewFrame(pf: " << (void*)pf << ")";

  cmd.issued = boost::posix_time::microsec_clock::local_time();

  boost::lock_guard<boost::mutex> guard(_mtx);
  _cmds.push(cmd);
}

COMMAND_T CmdQueue::get() {
  // BOOST_LOG_TRIVIAL(trace) << __LINE__ << "CmdQueue::getLastFrame()";
  
  boost::lock_guard<boost::mutex> guard(_mtx);
  if (_cmds.empty()) {
    return create_NO_COMMAND();
  }
  
  // @TODO: check TTL
  
  COMMAND_T cmd = _cmds.front();
  _cmds.pop();
  return cmd;
}
  void emergencyStop();
  void clear();

void CmdQueue::emergencyStop() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "CmdQueue::emergencyStop()";
  
  boost::lock_guard<boost::mutex> guard(_mtx);

  std::queue<COMMAND_T> empty;
  std::swap(_cmds, empty);
  
  _cmds.push(create_EMERGENCY_STOP());
}

void CmdQueue::clear() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "CmdQueue::clear()";
  
  boost::lock_guard<boost::mutex> guard(_mtx);

  std::queue<COMMAND_T> empty;
  std::swap(_cmds, empty);
}

  
  
