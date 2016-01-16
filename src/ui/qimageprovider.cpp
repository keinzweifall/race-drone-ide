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

#include <qt4/QtCore/QTimer>

#include "qimageprovider.h"
#include "model/videoframefeed.h"

QImageProvider::QImageProvider(VideoFrameFeed* pFeed, unsigned int pFrameRate) : _frameRate(pFrameRate), _feed(pFeed) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QImageProvider::QImageProvider()";
  
  _timer = nullptr;
  
  if (_feed == nullptr) {
    BOOST_LOG_TRIVIAL(error) << __LINE__ << "pFeed was null";
  }
}

QImageProvider::~QImageProvider() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QImageProvider::~QImageProvider()";
  if (_timer != nullptr) {
    stopUpdating();
  }
}

void QImageProvider::startUpdating() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QImageProvider::startUpdating()";
  
  if (_timer == nullptr) {
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(refresh()));
    _timer->setInterval(_frameRate);
    _timer->start();
  }
}

void QImageProvider::stopUpdating() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QImageProvider::stopUpdating()";

  if (_timer && _timer->isActive()) {
    _timer->stop();
    disconnect(_timer, SIGNAL(timeout()), this, SLOT(refresh()));
    delete _timer;
    _timer = nullptr;
  }
}

void QImageProvider::refresh() {
  // BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QImageProvider::refresh()";

  frameptr_t pf = _feed->getLastFrame(true);
  if (pf != nullptr) {
    _img.loadFromData(pf, _feed->getFrameSize(), "JPG");
    emit newImageAvailable();
  }
}

QPixmap QImageProvider::getCurrentImage() {
  // BOOST_LOG_TRIVIAL(trace) << __LINE__ << "QImageProvider::getCurrentImage()";
  return _img;
}
