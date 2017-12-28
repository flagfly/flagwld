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


#include <flagwld/net/EventLoopThreadPool.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/EventLoop.h>
#include <flagwld/net/EventLoopThread.h>

#include <boost/bind.hpp>

#include <stdio.h> //snprintf
#include <time.h> //time

using namespace flagwld;
using namespace flagwld::net;

void flagwld::net::defaultThreadInitCallback(const EventLoopPtr& loop)
{
  LOG_TRACE << "defaultThreadInitCallback(): pid = " << getpid() << " tid = " << CurrentThread::tid() << " loop= " << get_pointer(loop);
}

void flagwld::net::defaultThreadExitCallback(const EventLoopPtr& loop)
{
  LOG_TRACE << "defaultThreadExitCallback(): pid = " << getpid() << " tid = " << CurrentThread::tid() << " loop= " << get_pointer(loop);
}

EventLoopThreadPool::EventLoopThreadPool(const EventLoopPtr& baseLoop)
  : baseLoop_(CHECK_NOTNULL(get_pointer(baseLoop))?baseLoop:baseLoop),
    name_("EventLoopThreadPool"),
    started_(false),
    numThreads_(0),
    threadCpuAffinity_(false),
    next_(0)
{
  LOG_DEBUG << "EventLoopThreadPool::ctor[" << name_ << "] at " << this;
}

EventLoopThreadPool::EventLoopThreadPool(const EventLoopPtr& baseLoop, const string& nameArg)
  : baseLoop_(CHECK_NOTNULL(get_pointer(baseLoop))?baseLoop:baseLoop),
    name_(nameArg),
    started_(false),
    numThreads_(0),
    threadCpuAffinity_(false),
    next_(0)
{
  LOG_DEBUG << "EventLoopThreadPool::ctor[" << name_ << "] at " << this;
}

EventLoopThreadPool::~EventLoopThreadPool()
{
  LOG_DEBUG << "EventLoopThreadPool::dtor[" << name_ << "] at " << this;
  // Don't delete loop, it's stack variable
  if (numThreads_ == 0 && threadExitCallback_)
  {
    threadExitCallback_(baseLoop_);
  }
}

void EventLoopThreadPool::start()
{
  assert(!started_);
  baseLoop_->assertInLoopThread();

  started_ = true;

  for (int i = 0; i < numThreads_; ++i)
  {
    char buf[name_.size() + 32];
    snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
    EventLoopThread* t = new EventLoopThread(threadInitCallback_, threadExitCallback_, buf);

    threads_.push_back(t);
    loops_.push_back(t->startLoop());
  }

  if (threadCpuAffinity_)
  {
    setCpuAffinity();
  }
  
  if (numThreads_ == 0 && threadInitCallback_)
  {
    threadInitCallback_(baseLoop_);
  }
}

EventLoopPtr EventLoopThreadPool::getNextLoop()
{
  baseLoop_->assertInLoopThread();
  assert(started_);
  EventLoopPtr loop = baseLoop_;

  if (!loops_.empty())
  {
    // round-robin
    loop = loops_[next_];
    ++next_;
    if (implicit_cast<size_t>(next_) >= loops_.size())
    {
      next_ = 0;
    }
  }
  return loop;
}

EventLoopPtr EventLoopThreadPool::getLoopForHash(size_t hashCode)
{
  baseLoop_->assertInLoopThread();
  EventLoopPtr loop = baseLoop_;

  if (!loops_.empty())
  {
    loop = loops_[hashCode % loops_.size()];
  }
  return loop;
}

std::vector<EventLoopPtr> EventLoopThreadPool::getAllLoops()
{ 
  //baseLoop_->assertInLoopThread();
  assert(started_);
  if (loops_.empty())
  { 
    return std::vector<EventLoopPtr>(1, baseLoop_);
  }
  else
  { 
    return loops_;
  }
}

void EventLoopThreadPool::setCpuAffinity()
{
  long cpunum = sysconf(_SC_NPROCESSORS_CONF);

  int cpu_pos = 0;

  if (numThreads_>0)
  {
    size_t i;
    for (i=0; i<loops_.size(); ++i)
    {
      if (++cpu_pos >= cpunum)
      {
        cpu_pos = 0;
      }
      loops_[i]->queueInLoop(boost::bind(&flagwld::setThreadCpuAffinity, cpu_pos));
    }
  }
  else
  {
    cpu_pos = static_cast<int>(time(NULL) % cpunum);
    if (cpu_pos==0 && cpunum>=2)
    {
      cpu_pos = 1;
    }
    baseLoop_->queueInLoop(boost::bind(&flagwld::setThreadCpuAffinity, cpu_pos));
  }
}

