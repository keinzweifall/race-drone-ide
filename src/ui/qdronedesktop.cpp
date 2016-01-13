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

#include <qt4/QtCore/qthread.h>

#include <boost/log/trivial.hpp>

#include "controller/devicecontroller.h"
#include "ui/qdronedesktop.h"
#include "ui/qanimationcontrol.h"
#include "ui/qdrivecontrol.h"
#include "ui/qdronedashboard.h"

QDroneDesktop::QDroneDesktop() {
  setupUi(this);
  
  _dctrl = new DeviceController();

  _pDashboard = new QDroneDashboard(_dctrl, this->wCtrlContainer);
  _pDriveCtrl = new QDriveControl(_dctrl, this->wCtrlContainer);
  _pAnimCtrl = new QAnimationControl(_dctrl, this->wCtrlContainer);

  this->wCtrlContainer->layout()->addWidget(_pDashboard);
  this->wCtrlContainer->layout()->addWidget(_pDriveCtrl);
  this->wCtrlContainer->layout()->addWidget(_pAnimCtrl);

  connect(actionConnect, SIGNAL(triggered()), this, SLOT(connectDrone()));
  connect(actionDisonnect, SIGNAL(triggered()), this, SLOT(disconnectDrone()));
  connect(actionStart_Updating, SIGNAL(triggered()), _pDashboard, SLOT(startUpdating()));
  connect(actionStop_Updating, SIGNAL(triggered()), _pDashboard, SLOT(stopUpdating()));
}

void QDroneDesktop::closeEvent(QCloseEvent *event) {
  if (_dctrl->isConnected()) {
    _dctrl->exitControlLoop();
    _dctrl->disconnect();
  }
  delete _dctrl;
  _dctrl = nullptr;
}

void QDroneDesktop::connectDrone() {
  _dctrl->connect();
  _dctrl->enterControlLoop();
  usleep(50000);
  _dctrl->ping();
}

void QDroneDesktop::disconnectDrone() {
  _dctrl->exitControlLoop();
  _dctrl->disconnect();
}
