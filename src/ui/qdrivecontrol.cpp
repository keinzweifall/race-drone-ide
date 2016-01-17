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

#include "ui/qdrivecontrol.h"
#include "model/cmdqueue.h"
#include "controller/devicecontroller.h"

QDriveControl::QDriveControl(DeviceController* pdc, QWidget* parent) : _dctrl(pdc), QFrame(parent) {
  setupUi(this);

  this->setFrameShape(QFrame::Box);
  
  connect(pbForward, SIGNAL(pressed()), this, SLOT(forwardPressed()));
  connect(pbForward, SIGNAL(released()), this, SLOT(forwardReleased()));
  connect(pbBackward, SIGNAL(pressed()), this, SLOT(backwardPressed()));
  connect(pbBackward, SIGNAL(released()), this, SLOT(backwardReleased()));
  connect(pbLeft, SIGNAL(pressed()), this, SLOT(leftPressed()));
  connect(pbLeft, SIGNAL(released()), this, SLOT(leftReleased()));
  connect(pbRight, SIGNAL(pressed()), this, SLOT(rightPressed()));
  connect(pbRight, SIGNAL(released()), this, SLOT(rightReleased()));
  connect(pbStop, SIGNAL(clicked(bool)), this, SLOT(stopClicked()));
  
  _cmdq = new CmdQueue(100);
  _dctrl->setCmdQueue(_cmdq);
}
  
void QDriveControl::forwardPressed() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot forwardPressed";
  COMMAND_T cmd = create_PILOTING(1, 50, 0);
  _cmdq->put(cmd);
}

void QDriveControl::forwardReleased() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot forwardReleased";
}

void QDriveControl::backwardPressed() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot backwardPressed";
  COMMAND_T cmd = create_PILOTING(1, -50, 0);
  _cmdq->put(cmd);
}

void QDriveControl::backwardReleased() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot backwardReleased";
}

void QDriveControl::leftPressed() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot leftPressed";
  COMMAND_T cmd = create_PILOTING(1, 0, -5);
  _cmdq->put(cmd);
}

void QDriveControl::leftReleased() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot leftReleased";
}

void QDriveControl::rightPressed() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot rightPressed";
  COMMAND_T cmd = create_PILOTING(1, 0, 5);
  _cmdq->put(cmd);
}

void QDriveControl::rightReleased() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot rightReleased";
}

void QDriveControl::stopClicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot stopClicked";
  COMMAND_T cmd = create_PILOTING(0, 0, 0);
  _cmdq->put(cmd);
}
