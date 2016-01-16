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

VideoFrameFeed::VideoFrameFeed(uint32_t pFrameSize, unsigned long pMaxFrames) {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "VideoFrameFeed::VideoFrameFeed(pFrameSize: " << pFrameSize << ", pMaxFrames: " << pMaxFrames << ")";
  
  _frameSize = pFrameSize;
  _maxFrames = pMaxFrames;
}

VideoFrameFeed::~VideoFrameFeed() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "VideoFrameFeed::~VideoFrameFeed()";

  boost::lock_guard<boost::mutex> guard(_mtx);

  while (!_frames.empty()) {
    frameptr_t f = _frames.front();
    _frames.pop();
    free(f);
  }
}

void VideoFrameFeed::setNewFrame(frameptr_t pf) {
  // BOOST_LOG_TRIVIAL(trace) << __LINE__ << "VideoFrameFeed::setNewFrame(pf: " << (void*)pf << ")";
  
  boost::lock_guard<boost::mutex> guard(_mtx);
  
  if (_frames.size() == _maxFrames) {
    _frames.pop();
  }

  frameptr_t f = (frameptr_t) std::malloc(_frameSize);
  std::memcpy(f, pf, _frameSize);
  _frames.push(f);
}

frameptr_t VideoFrameFeed::getLastFrame(bool pConsuming) {
  // BOOST_LOG_TRIVIAL(trace) << __LINE__ << "VideoFrameFeed::getLastFrame(pConsuming: " << pConsuming << ")";
  
  boost::lock_guard<boost::mutex> guard(_mtx);
  
  if (_frames.empty()) {
    return nullptr;
  }
  
  frameptr_t f = _frames.front();
  
  if (pConsuming) {
    _frames.pop();
  }
  
  return f;
}

unsigned int VideoFrameFeed::getFrameCount() {
  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "VideoFrameFeed::getFrameCount()";
  
  boost::lock_guard<boost::mutex> guard(_mtx);

  return _frames.size();
}
  
  
