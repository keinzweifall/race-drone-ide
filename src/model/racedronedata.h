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

#ifndef RACEDRONEDATA_H
#define RACEDRONEDATA_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif


struct RACEDRONE_VALUES {
  bool connected;
  bool alert;
  float batteryChargePercentage;
  int speedInCmPerSec;
  int speedVal;
  int turnVal;
  int linkQuality; // 0 - 6, the higher, the better
  std::string posture;
};

typedef struct RACEDRONE_VALUES RACEDRONE_VALUES_T;

#ifdef __cplusplus 
}
#endif

#endif // RACEDRONEDATA_H
