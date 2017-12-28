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


#ifndef FLAGWLD_BASE_ASYNCLOGGING_H
#define FLAGWLD_BASE_ASYNCLOGGING_H

#include <flagwld/base/BlockingQueue.h>
#include <flagwld/base/BoundedBlockingQueue.h>
#include <flagwld/base/CountDownLatch.h>
#include <flagwld/base/Mutex.h>
#include <flagwld/base/Thread.h>
#include <flagwld/base/LogStream.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace flagwld
{

class AsyncLogging : boost::noncopyable
{
 public:

  AsyncLogging( const string& dirname,
                const string& basename,
               size_t rollSize,
               int flushInterval = 3);

  ~AsyncLogging()
  {
    if (running_)
    {
      stop();
    }
  }

  void append(const char* logline, int len);

  void start()
  {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop()
  {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

 private:

  // declare but not define, prevent compiler-synthesized functions
  AsyncLogging(const AsyncLogging&);  // ptr_container
  void operator=(const AsyncLogging&);  // ptr_container

  void threadFunc();

  //typedef flagwld::detail::FixedBuffer<flagwld::detail::kLargeBuffer> Buffer;
  typedef flagwld::detail::FixedBuffer<flagwld::detail::kSmallBuffer> Buffer;
  typedef boost::ptr_vector<Buffer> BufferVector;
  typedef BufferVector::auto_type BufferPtr;

  const int flushInterval_;
  bool running_;
  string dirname_;
  string basename_;
  size_t rollSize_;
  flagwld::Thread thread_;
  flagwld::CountDownLatch latch_;
  flagwld::MutexLock mutex_;
  flagwld::Condition cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
};

}
#endif  // FLAGWLD_BASE_ASYNCLOGGING_H
