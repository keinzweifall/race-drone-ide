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

#ifndef QDRONEDASHBOARD_H
#define QDRONEDASHBOARD_H

#include <string.h>

#include <qt4/QtGui/QFrame>

#include "ui_qdronedashboard.h"

class DeviceController;
class QTimer;

class QDroneDashboard : public QFrame, public Ui::QDroneDashboard
{
    Q_OBJECT
    
public:
    QDroneDashboard(DeviceController* pdc, QWidget* parent = 0);
    
private slots:
  void startUpdating();
  void stopUpdating();
  void refresh();

private:
  QTimer* _timer;
  DeviceController* _dctrl;

private:
  static int cREFRESH_INTERVAL;
};

#endif // QDRONEDASHBOARD_H
