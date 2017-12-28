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


#ifndef FLAGWLD_NET_EVENTLOOPTHREAD_H
#define FLAGWLD_NET_EVENTLOOPTHREAD_H

#include <flagwld/base/Condition.h>
#include <flagwld/base/Mutex.h>
#include <flagwld/base/Thread.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace flagwld
{
namespace net
{
class EventLoop;
typedef boost::shared_ptr<EventLoop> EventLoopPtr;

class EventLoopThread : boost::noncopyable
{
 public:
  typedef boost::function<void(EventLoopPtr&)> ThreadInitCallback;
  typedef boost::function<void(EventLoopPtr&)> ThreadExitCallback;

  EventLoopThread(const ThreadInitCallback& initcb = ThreadInitCallback(),
                 const ThreadExitCallback& exitcb = ThreadExitCallback(),
                 const string& name = string("EventLoopThread"));
  ~EventLoopThread();
  EventLoopPtr startLoop();

 private:
  void threadFunc();

  EventLoopPtr loop_;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
  ThreadInitCallback threadInitCallback_;
  ThreadExitCallback threadExitCallback_;
};

}
}

#endif  // FLAGWLD_NET_EVENTLOOPTHREAD_H

