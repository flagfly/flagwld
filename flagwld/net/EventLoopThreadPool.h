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


#ifndef FLAGWLD_NET_EVENTLOOPTHREADPOOL_H
#define FLAGWLD_NET_EVENTLOOPTHREADPOOL_H

#include <flagwld/base/Types.h>

#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

namespace flagwld
{

namespace net
{

class EventLoop;
class EventLoopThread;
typedef boost::shared_ptr<EventLoop> EventLoopPtr;

void defaultThreadInitCallback(const EventLoopPtr&);
void defaultThreadExitCallback(const EventLoopPtr&);

class EventLoopThreadPool : boost::noncopyable
{
 public:
  typedef boost::function<void(const EventLoopPtr&)> ThreadInitCallback;
  typedef boost::function<void(const EventLoopPtr&)> ThreadExitCallback;

  EventLoopThreadPool(const EventLoopPtr& baseLoop);
  EventLoopThreadPool(const EventLoopPtr& baseLoop, const string& nameArg);
  ~EventLoopThreadPool();

  void setThreadNum(int numthreads) { numThreads_ = numthreads; }
  void setThreadCpuAffinity(bool on) { threadCpuAffinity_ = on; }
  void setThreadInitCallback(const ThreadInitCallback& cb)
  { threadInitCallback_ = cb; }
  void setThreadExitCallback(const ThreadExitCallback& cb)
  { threadExitCallback_ = cb; }

  void start();

  const string& name() const
  { return name_; }

  inline EventLoopPtr getLoop() const
  { return baseLoop_; }

  inline int numThreads() const
  { return numThreads_; }

  // valid after calling start()
  /// round-robin
  EventLoopPtr getNextLoop();
  /// with the same hash code, it will always return the same EventLoop
  EventLoopPtr getLoopForHash(size_t hashCode);
  std::vector<EventLoopPtr> getAllLoops();

  bool started() const
  { return started_; }

 private:
  void setCpuAffinity();

 private:
  EventLoopPtr baseLoop_;
  string name_;
  bool started_;
  int numThreads_;
  bool threadCpuAffinity_;
  int next_;
  boost::ptr_vector<EventLoopThread> threads_;
  std::vector<EventLoopPtr> loops_;
  ThreadInitCallback threadInitCallback_;
  ThreadExitCallback threadExitCallback_;
};

}
}

#endif  // FLAGWLD_NET_EVENTLOOPTHREADPOOL_H
