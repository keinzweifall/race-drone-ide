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

#include <unistd.h>

#include <boost/log/trivial.hpp>

#include <qt4/QtCore/QTimer>

#include "controller/devicecontroller.h"
#include "ui/qdronedashboard.h"

int QDroneDashboard::cREFRESH_INTERVAL = 100;

QDroneDashboard::QDroneDashboard(QWidget* parent) : QFrame(parent) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QDroneDashboard::QDroneDashboard(QWidget* parent)";
  
  setupUi(this);
  this->setFrameShape(QFrame::Box);
  _dctrl = new DeviceController();
  _timer = nullptr;
}

void QDroneDashboard::startUpdating() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QDroneDashboard::startUpdating()";
  
  if (!_timer) {
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(refresh()));
    _timer->setInterval(cREFRESH_INTERVAL);
    _timer->start();
  }
}

void QDroneDashboard::stopUpdating() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QDroneDashboard::stopUpdating()";

  if (_timer && _timer->isActive()) {
    _timer->stop();
    disconnect(_timer, SIGNAL(timeout()), this, SLOT(refresh()));
    delete _timer;
    _timer = NULL;
  }
}

void QDroneDashboard::refresh() {
  // BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QDroneDashboard::refresh()";

  RACEDRONE_VALUES_T pv = _dctrl->getValues();

  rbConnectionState->setChecked(pv.connected);
  rbAlertState->setChecked(pv.alert);
  lcdCharge->display(pv.batteryChargePercentage);
  lcdSpeedCMS->display(pv.speedInCmPerSec);
  lcdSpeedVal->display(pv.speedVal);
  lcdTurnVal->display(pv.turnVal);
  lcdLinkQuality->display(pv.linkQuality);
  lePosture->clear();
  lePosture->insert(QString::fromStdString(pv.posture));
}
