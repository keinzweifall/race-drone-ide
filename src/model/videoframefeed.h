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

#ifndef VIDEOFRAMEFEED_H
#define VIDEOFRAMEFEED_H

#include <vector>

#include <boost/thread.hpp>

typedef uint8_t* frameptr_t;

class VideoFrameFeed
{
public:
  VideoFrameFeed(unsigned int pFrameSize, unsigned long pMaxFrames);
  ~VideoFrameFeed();
  
  void setNewFrame(frameptr_t pf);
  frameptr_t getLastFrame();
  unsigned int getFrameCount();

  unsigned int getFrameSize() { return _frameSize; }
  
private:
  std::vector<frameptr_t> _ringBuf;
  unsigned int _lastInsertPos;
  unsigned long _frameCnt;
  unsigned int _frameSize;
  unsigned long _maxFrames;
  boost::mutex _mtx;
};

#endif // VIDEOFRAMEFEED_H
