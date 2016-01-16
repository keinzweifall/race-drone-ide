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

#include "ui/qvideodisplay.h"
#include "ui/qimageprovider.h"
#include "controller/devicecontroller.h"

QVideoDisplay::QVideoDisplay(DeviceController* pdc, QWidget* parent) : _dctrl(pdc), QWidget(parent) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QVideoDisplay::QVideoDisplay()";

  setupUi(this);

  int frameRate = 1000 / 24; // 24 frames per second

  VideoFrameFeed* feed = _dctrl->startVideo();
  _imgProvider = new QImageProvider(feed, frameRate);
  connect(_imgProvider, SIGNAL(newImageAvailable()), this, SLOT(setNewImage()));
  _imgProvider->startUpdating();
}

QVideoDisplay::~QVideoDisplay() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QVideoDisplay::~QVideoDisplay()";

  _imgProvider->stopUpdating();
  disconnect(_imgProvider, SIGNAL(newImageAvailable()), this, SLOT(setNewImage()));
}


void QVideoDisplay::setNewImage(){
  // BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QVideoDisplay::setNewImage()";

  lVideoImage->setPixmap(_imgProvider->getCurrentImage());
  update();
}
