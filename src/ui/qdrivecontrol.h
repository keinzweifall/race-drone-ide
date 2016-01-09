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

#ifndef QDRIVECONTROL_H
#define QDRIVECONTROL_H

#include <QFrame>

#include "ui_qdrivecontrol.h"


class QDriveControl : public QFrame, public Ui::QDriveControl
{
    Q_OBJECT
public:
    QDriveControl(QWidget* parent = 0);
  
public slots:
    void forwardPressed();
    void forwardReleased();
    void backwardPressed();
    void backwardReleased();
    void leftPressed();
    void leftReleased();
    void rightPressed();
    void rightReleased();
    void stopClicked();
signals:

private:
    bool _forward;
    bool _backward;
    bool _right;
    bool _left;
};

#endif // QDRIVECONTROL_H
