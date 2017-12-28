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


#include <flagwld/base/LogFile.h>
#include <flagwld/base/FileUtil.h>
#include <flagwld/base/Logging.h> // strerror_tl
#include <flagwld/base/TimeZone.h>
#include <flagwld/base/ProcessInfo.h>

#include <assert.h>
#include <stdio.h>
#include <time.h>

namespace flagwld 
{
  extern TimeZone g_logTimeZone;
}

using namespace flagwld;

LogFile::LogFile(const string& dirname,
                 const string& basename,
                 size_t rollSize,
                 bool threadSafe,
                 int flushInterval,
                 int checkEveryN)
  : dirname_(dirname),
    basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    checkEveryN_(checkEveryN),
    checked_(0),
    mutex_(threadSafe ? new MutexLock : NULL),
    lastFlush_(0)
{
  assert(basename.find('/') == string::npos);
  assert(!dirname_.empty());

  bzero(&startOfPeriod_, sizeof(startOfPeriod_));
  bzero(&lastRoll_, sizeof(lastRoll_));

  time_t now = Timestamp::now().secondsSinceEpoch();
  struct tm nowtm;
  if (flagwld::g_logTimeZone.valid())
  {
    nowtm = flagwld::g_logTimeZone.toLocalTime(now);
  }
  else
  {
    ::gmtime_r(&now, &nowtm); // FIXME: localtime_r ?
  }

  rollFile(&nowtm);
}

LogFile::~LogFile()
{
}

void LogFile::append(const char* logline, int len)
{
  if (mutex_)
  {
    MutexLockGuard lock(*mutex_);
    append_unlocked(logline, len);
  }
  else
  {
    append_unlocked(logline, len);
  }
}

void LogFile::flush()
{
  if (mutex_)
  {
    MutexLockGuard lock(*mutex_);
    file_->flush();
  }
  else
  {
    file_->flush();
  }
}

void LogFile::append_unlocked(const char* logline, int len)
{
  if (!file_)
  {
    fprintf(stderr, "LogFile::append() failed open error.\n");
    return ;
  }

  time_t now = Timestamp::now().secondsSinceEpoch();

  if (file_->writtenBytes() > rollSize_)
  {
    struct tm nowtm;
    if (flagwld::g_logTimeZone.valid())
    {
      nowtm = flagwld::g_logTimeZone.toLocalTime(now);
    }
    else
    {
      ::gmtime_r(&now, &nowtm); // FIXME: localtime_r ?
    }

    if (!rollFile(&nowtm))
    {
      fprintf(stderr, "LogFile::append() failed roll error.\n");
      return ;
    }
  }
  else
  {
    if (now>=(checked_+checkEveryN_))
    {
      checked_ = now;

      struct tm nowtm;
      if (flagwld::g_logTimeZone.valid())
      {
        nowtm = flagwld::g_logTimeZone.toLocalTime(now);
      }
      else
      {
        ::gmtime_r(&now, &nowtm); // FIXME: localtime_r ?
      }

      if (nowtm.tm_mday>startOfPeriod_.tm_mday||
          nowtm.tm_mon>startOfPeriod_.tm_mon||
          nowtm.tm_year>startOfPeriod_.tm_year)
      {
        if (!rollFile(&nowtm))
        {
          fprintf(stderr, "LogFile::append() failed roll error.\n");
          return ;
        }
      }
      else if (now - lastFlush_ > flushInterval_)
      {
        lastFlush_ = now;
        file_->flush();
      }
    }
  }

  file_->append(logline, len);
}

bool LogFile::rollFile(struct tm* nowtm)
{
  time_t nowsec = ::mktime(nowtm);

  string filename = getLogFileName(nowtm, dirname_, basename_);
  file_.reset(new FileUtil::AppendFile(filename));
  if (!file_)
  {
    return false;
  }

  lastRoll_ = *nowtm;
  startOfPeriod_ = *nowtm;
  checked_ = nowsec;
  lastFlush_ = nowsec;

  return true;
}

string LogFile::getLogFileName(struct tm* nowtm, const string&dirname, const string& basename)
{
  string filename;

  filename.reserve(dirname.size() + 1 + basename.size() + 64);
  if (dirname[dirname.length()-1] != '/')
  {
    filename = dirname + "/" + basename;
  }
  else
  {
    filename = dirname + basename;
  }

  char timebuf[32];

  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", nowtm);
  filename += timebuf;
  filename += ProcessInfo::hostname();

  char pidbuf[32];
  snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());
  filename += pidbuf;

  filename += ".log";

  return filename;
}
