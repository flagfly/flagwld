/*
   Copyright (C) Zhang GuoQi <guoqi.zhang@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   see https://github.com/chenshuo/muduo/
   see http://software.schmorp.de/pkg/libev.html

   libev was written and designed by Marc Lehmann and Emanuele Giaquinta.
   muduo was written by chenshuo.
*/


#ifndef FLAGWLD_BASE_LOGFILE_H
#define FLAGWLD_BASE_LOGFILE_H

#include <flagwld/base/Mutex.h>
#include <flagwld/base/Types.h>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace flagwld
{

namespace FileUtil
{
class AppendFile;
}

class LogFile : boost::noncopyable
{
 public:
  LogFile(const string& dirname,
          const string& basename,
          size_t rollSize,
          bool threadSafe = true,
          int flushInterval = 3,
          int checkEveryN = 1);
  ~LogFile();

  void append(const char* logline, int len);
  void flush();
 private:
  void append_unlocked(const char* logline, int len);

  bool rollFile(struct tm* nowtm);
  static string getLogFileName(struct tm* nowtm, const string& dirname, const string& basename);

  const string dirname_;
  const string basename_;
  const size_t rollSize_;
  const int flushInterval_;
  const int checkEveryN_;

  time_t checked_;

  boost::scoped_ptr<MutexLock> mutex_;
  struct tm startOfPeriod_;
  struct tm lastRoll_;
  time_t lastFlush_;
  boost::scoped_ptr<FileUtil::AppendFile> file_;
};

}
#endif  // FLAGWLD_BASE_LOGFILE_H
