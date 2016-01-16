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

#ifndef QVIDEODISPLAY_H
#define QVIDEODISPLAY_H

#include <qt4/QtGui/QWidget>

#include "ui_qvideodisplay.h"

class DeviceController;
class QImageProvider;

class QVideoDisplay : public QWidget, Ui::QVideoDisplay
{
  Q_OBJECT

public:
  QVideoDisplay(DeviceController* pdc, QWidget* parent = 0);
  ~QVideoDisplay();
  
private slots:
  void setNewImage();

private:
  DeviceController* _dctrl;
  QImageProvider* _imgProvider;
};

#endif // QVIDEODISPLAY_H
