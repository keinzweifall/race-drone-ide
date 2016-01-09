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

#include "ui/qdronedesktop.h"
#include "ui/qanimationcontrol.h"
#include "ui/qdrivecontrol.h"
#include "ui/qdronedashboard.h"

QDroneDesktop::QDroneDesktop() {
  setupUi(this);

  _pDashboard = new QDroneDashboard(this->wCtrlContainer);
  _pDriveCtrl = new QDriveControl(this->wCtrlContainer);
  _pAnimCtrl = new QAnimationControl(this->wCtrlContainer);

  this->wCtrlContainer->layout()->addWidget(_pDashboard);
  this->wCtrlContainer->layout()->addWidget(_pDriveCtrl);
  this->wCtrlContainer->layout()->addWidget(_pAnimCtrl);
  
  connect(actionStart_Updating, SIGNAL(triggered()), _pDashboard, SLOT(startUpdating()));
  connect(actionStop_Updating, SIGNAL(triggered()), _pDashboard, SLOT(stopUpdating()));
}

void QDroneDesktop::closeEvent(QCloseEvent *event) {
}

