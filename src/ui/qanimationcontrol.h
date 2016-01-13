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

#ifndef QANIMATIONCONTROL_H
#define QANIMATIONCONTROL_H

#include <qt4/QtGui/QFrame>

#include "ui_qanimationcontrol.h"

class DeviceController;

class QAnimationControl : public QFrame, public Ui::QAnimationControl
{
    Q_OBJECT
public:
    QAnimationControl(DeviceController* pdc, QWidget* parent = 0);
    
private slots:
  void on_pbJumpWide_clicked();
  void on_pbBoost_clicked();
  void on_pbTap_clicked();
  void on_pbJumpHigh_clicked();
  void on_pbSpin2Posture_clicked();
  void on_pbSlowShake_clicked();
  void on_pbSpinJump_clicked();
  void on_pbSpiral_clicked();
  void on_pbMetronome_clicked();
  void on_pbSpin_clicked();
  void on_pbOndulation_clicked();
  void on_pbSlalom_clicked();

private:
  DeviceController* _dctrl;

};

#endif // QANIMATIONCONTROL_H
