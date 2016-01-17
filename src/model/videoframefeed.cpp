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

#include <cstring>

#include <boost/log/trivial.hpp>
#include <boost/concept_check.hpp>

#include "videoframefeed.h"

VideoFrameFeed::VideoFrameFeed(uint32_t pFrameSize, unsigned long pMaxFrames) : _ringBuf(pMaxFrames) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "VideoFrameFeed::VideoFrameFeed(pFrameSize: " << pFrameSize << ", pMaxFrames: " << pMaxFrames << ")";
  
  _lastInsertPos = _frameCnt = 0;
  _frameSize = pFrameSize;
  _maxFrames = pMaxFrames;
  
  // pre-allocate ring buffer for frames
  for (std::vector<frameptr_t>::iterator i = _ringBuf.begin(); i < _ringBuf.end(); i++) {
    *i = (frameptr_t) std::malloc(_frameSize);
  }
}

VideoFrameFeed::~VideoFrameFeed() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "VideoFrameFeed::~VideoFrameFeed()";

  boost::lock_guard<boost::mutex> guard(_mtx);

  for (auto f : _ringBuf) {
    free(f);
  }
}

void VideoFrameFeed::setNewFrame(frameptr_t pf) {
  // BOOST_LOG_TRIVIAL(trace) << __LINE__ << "VideoFrameFeed::setNewFrame(pf: " << (void*)pf << ")";
  
  boost::lock_guard<boost::mutex> guard(_mtx);
  
  if (++_lastInsertPos == _maxFrames) {
    _lastInsertPos = 0;
  }
  
  if (_frameCnt < _maxFrames) {
    _frameCnt++;
  }
  
  std::memcpy(_ringBuf[_lastInsertPos], pf, _frameSize);
}

frameptr_t VideoFrameFeed::getLastFrame() {
  // BOOST_LOG_TRIVIAL(trace) << __LINE__ << "VideoFrameFeed::getLastFrame()";
  
  boost::lock_guard<boost::mutex> guard(_mtx);
  
  if (_frameCnt == 0) {
    return nullptr;
  }
  
  return _ringBuf[_lastInsertPos];
}

unsigned int VideoFrameFeed::getFrameCount() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "VideoFrameFeed::getFrameCount()";
  
  boost::lock_guard<boost::mutex> guard(_mtx);

  return _frameCnt;
}
  
  
