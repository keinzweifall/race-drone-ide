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

QDriveControl::QDriveControl(QWidget* parent) : QFrame(parent) {
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
}
  
void QDriveControl::forwardPressed() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot forwardPressed";
  _forward = true;
}

void QDriveControl::forwardReleased() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot forwardReleased";
  _forward = false;
}

void QDriveControl::backwardPressed() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot backwardPressed";
  _backward = true;
}

void QDriveControl::backwardReleased() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot backwardReleased";
  _backward = false;
}

void QDriveControl::leftPressed() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot leftPressed";
  _left = true;
}

void QDriveControl::leftReleased() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot leftReleased";
  _left = false;
}

void QDriveControl::rightPressed() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot rightPressed";
  _right = true;
}

void QDriveControl::rightReleased() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot rightReleased";
  _right = false;
}

void QDriveControl::stopClicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot stopClicked";
  _forward = _backward = _left = _right = false;
}
