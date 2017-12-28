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


#include <flagwld/net/EventLoopThread.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace flagwld;
using namespace flagwld::net;


EventLoopThread::EventLoopThread(const ThreadInitCallback& initcb,
   const ThreadExitCallback& exitcb,
   const string& name)
  : exiting_(false),
    thread_(boost::bind(&EventLoopThread::threadFunc, this), name),
    mutex_(),
    cond_(mutex_),
    threadInitCallback_(initcb),
    threadExitCallback_(exitcb)
{
  LOG_DEBUG << "EventLoopThread::ctor[" << thread_.name() << "] at " << this;
}

EventLoopThread::~EventLoopThread()
{
  LOG_DEBUG << "EventLoopThread::dtor[" << thread_.name() << "] at " << this;
  exiting_ = true;
  if (loop_)
  {
    loop_->quit();
    thread_.join();
  }
}

EventLoopPtr EventLoopThread::startLoop()
{
  assert(!thread_.started());
  thread_.start();

  {
    MutexLockGuard lock(mutex_);
    while (!loop_)
    {
      cond_.wait();
    }
  }

  return loop_;
}

void EventLoopThread::threadFunc()
{
  EventLoopPtr loop(new EventLoop);

  if (threadInitCallback_)
  {
    threadInitCallback_(loop);
  }

  {
    MutexLockGuard lock(mutex_);
    loop_.swap(loop);
    cond_.notify();
  }

  loop_->loop();
  //assert(exiting_);

  if (threadExitCallback_)
  {
    threadExitCallback_(loop_);
  }
}

