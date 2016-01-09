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
    keywords::file_name = "race-drone-dev-%N.log",
    keywords::rotation_size = 10 * 1024 * 1024,
    keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
    keywords::format = "[%ThreadID%][%TimeStamp%]: %Message%"
    // keywords::format = "[%LineID%][%ProcessID%.%ThreadID%][%TimeStamp%]: %Message%"
  );

  logging::core::get()->set_filter  (
    logging::trivial::severity >= logging::trivial::trace
  );
}

/*
  const char * test = "{\"project\":\"rapidjson\",\"stars\":10}";;

  rapidjson::Document d;
  d.Parse(test);
  rapidjson::Value &v = d["stars"];
  std::cout << "v=" << v.GetInt() << std::endl;
  
  rapidjson::StringBuffer s;
  rapidjson::Writer<rapidjson::StringBuffer> writer(s);
  
  writer.StartObject();
  writer.String("hello");
  writer.String("world");
  writer.String("t");
  writer.Bool(true);
  writer.String("f");
  writer.Bool(false);
  writer.String("n");
  writer.Null();
  writer.String("i");
  writer.Uint(123);
  writer.String("pi");
  writer.Double(3.1416);
  writer.String("a");
  writer.StartArray();
  for (unsigned i = 0; i < 4; i++)
      writer.Uint(i);
  writer.EndArray();
  writer.EndObject();

  std::cout << s.GetString() << std::endl;

  return 0;
  
 */

int main(int argc, char** argv)
{
  init_logging();
  logging::add_common_attributes();

  QApplication app(argc, argv);
  QDroneDesktop desktop;
  desktop.show();

  int lRetval = app.exec();

  BOOST_LOG_TRIVIAL(trace) << __LINE__ << "lRetval=" << lRetval;

  return lRetval;
}
