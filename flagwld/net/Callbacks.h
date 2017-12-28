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


#ifndef FLAGWLD_NET_CALLBACKS_H
#define FLAGWLD_NET_CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <flagwld/base/Types.h>

namespace flagwld
{

// Adapted from google-protobuf stubs/common.h
// see License in flagwld/base/Types.h
template<typename To, typename From>
inline ::boost::shared_ptr<To> down_pointer_cast(const ::boost::shared_ptr<From>& f)
{
  if (false)
  {
    implicit_cast<From*, To*>(0);
  }

#ifndef NDEBUG
  assert(f == NULL || dynamic_cast<To*>(get_pointer(f)) != NULL);
#endif
  return ::boost::static_pointer_cast<To>(f);
}

namespace net
{

// All client visible callbacks go here.

typedef boost::function<void()> TimerCallback;
typedef boost::function<void()> PeriodicCallback;
typedef boost::function<double(double)> RescheduleCallback;
typedef boost::function<void()> AsynCallback;
typedef boost::function<void()> IdleCallback;
typedef boost::function<void()> SignalCallback;
typedef boost::function<void(pid_t, int)> ChildCallback;

class Buffer;

/******************** tcp ********************/
class SockAddress;
class SockConnection;

typedef boost::shared_ptr<SockConnection> SockConnectionPtr;
typedef boost::function<void (const SockConnectionPtr&)> ConnectionCallback;
typedef boost::function<bool(const flagwld::net::SockAddress&)> ConnectErrorCallback;
typedef boost::function<void (const SockConnectionPtr&)> ConnectionIdleCallback;
typedef boost::function<void (const SockConnectionPtr&)> WriteCompleteCallback;
typedef boost::function<void (const SockConnectionPtr&, size_t)> HighWaterMarkCallback;
typedef boost::function<void (const SockConnectionPtr&)> CloseCallback;

// the data has been read to (buf, len)
typedef boost::function<void (const SockConnectionPtr&, Buffer*)> MessageCallback;

bool defaultConnectErrorCallback(const flagwld::net::SockAddress&);
void defaultConnectionIdleCallback(const SockConnectionPtr& conn);
void defaultConnectionCallback(const SockConnectionPtr& conn);
void defaultMessageCallback(const SockConnectionPtr& conn, Buffer* buffer);
void defaultWriteCompleteCallback(const SockConnectionPtr& conn);
void defaultHighWaterMarkCallback(const SockConnectionPtr& conn, size_t);

}
}

#endif  // FLAGWLD_NET_CALLBACKS_H
