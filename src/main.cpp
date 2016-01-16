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

#include <signal.h>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <QtGui/QApplication>

#include "ui/qdronedesktop.h"
#include "controller/devicecontroller.h"

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;


void init_logging()
{
  logging::add_file_log (
    keywords::file_name = "race-drone-ide-%N.log",
    keywords::rotation_size = 10 * 1024 * 1024,
    keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
    keywords::auto_flush = true,
    keywords::format = "[%ThreadID%][%TimeStamp%]: %Message%"
    // keywords::format = "[%LineID%][%ProcessID%.%ThreadID%][%TimeStamp%]: %Message%"
  );

  logging::core::get()->set_filter  (
    logging::trivial::severity >= logging::trivial::trace
  );
}

int main(int argc, char** argv)
{
  init_logging();
  logging::add_common_attributes();

  /*
  pid_t child = 0;

  // fork the process to launch ffplay
  if ((child = fork()) == 0)
  {
    // execlp("ffplay", "ffplay", "-i", "video_fifo", "-f", "mjpeg", NULL);
    execlp("avplay", "avplay", "-i", "video_fifo", "-f", "mjpeg", NULL);
    return -1;
  }
  */

  QApplication app(argc, argv);
  QDroneDesktop desktop;
  desktop.show();

  int lRetval = app.exec();

  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "lRetval=" << lRetval;

  /*
  if (child > 0)
  {
    kill(child, SIGKILL);
  }
  */

  return lRetval;
}
