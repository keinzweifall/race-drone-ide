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

#ifndef QDRONEDESKTOP_H
#define QDRONEDESKTOP_H

#include <qt4/QtGui/QMainWindow>
#include <boost/concept_check.hpp>

#include "model/racedronedata.h"
#include "ui_qdronedesktop.h"

class QAnimationControl;
class QDriveControl;
class QDroneDashboard;
class DeviceController;

class QDroneDesktop : public QMainWindow, public Ui::QDroneDesktop
{
  Q_OBJECT
public:
  QDroneDesktop();

protected:
  void closeEvent(QCloseEvent *event);
  
private slots:
  void connectDrone();
  void disconnectDrone();
    
private:
  QAnimationControl* _pAnimCtrl;
  QDriveControl* _pDriveCtrl;
  QDroneDashboard* _pDashboard;
  DeviceController* _dctrl;
};

#endif // QDRONEDESKTOP_H
