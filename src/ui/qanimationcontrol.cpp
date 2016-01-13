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

#include "ui/qanimationcontrol.h"

QAnimationControl::QAnimationControl(DeviceController* pdc, QWidget* parent) : _dctrl(pdc), QFrame(parent) {
  setupUi(this);
  this->setFrameShape(QFrame::Box);
}

void QAnimationControl::on_pbJumpWide_clicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot on_pbJumpWide_clicked";
}

void QAnimationControl::on_pbBoost_clicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot on_pbBoost_clicked";
}

void QAnimationControl::on_pbTap_clicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot on_pbTap_clicked";
}

void QAnimationControl::on_pbJumpHigh_clicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot on_pbJumpHigh_clicked";
}

void QAnimationControl::on_pbSpin2Posture_clicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot on_pbSpin2Posture_clicked";
}

void QAnimationControl::on_pbSlowShake_clicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot on_pbSlowShake_clicked";
}

void QAnimationControl::on_pbSpinJump_clicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot on_pbSpinJump_clicked";
}

void QAnimationControl::on_pbSpiral_clicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot on_pbSpiral_clicked";
}

void QAnimationControl::on_pbMetronome_clicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot on_pbMetronome_clicked";
}

void QAnimationControl::on_pbSpin_clicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot on_pbSpin_clicked";
}

void QAnimationControl::on_pbOndulation_clicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot on_pbOndulation_clicked";
}

void QAnimationControl::on_pbSlalom_clicked() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << " slot on_pbSlalom_clicked";
}
