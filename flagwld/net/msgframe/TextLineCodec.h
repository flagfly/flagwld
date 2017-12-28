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


#ifndef __FLAGWLD_NET_MSG_TEXTLINECODEC_H_
#define __FLAGWLD_NET_MSG_TEXTLINECODEC_H_

#include <flagwld/net/msgframe/Message.h>
#include <flagwld/net/msgframe/TextLineParser.h>

#include <flagwld/net/TcpConnection.h>

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

#include <sys/types.h>

namespace flagwld
{
namespace net
{

typedef boost::function<void (const SockConnectionPtr&, const char*, size_t)> LineMsgCallback;

class TextLineCodec : boost::noncopyable
{
public:
  TextLineCodec();
  ~TextLineCodec();

  void setLineMsgCallback(const LineMsgCallback&cb)
  { lineMsgCallback_=cb; }

  inline void setCompatible(bool f) { lineParser_.setCompatible(f); }

  void onMessage(const flagwld::net::SockConnectionPtr& conn, flagwld::net::Buffer* buf);

private:
  LineMsgCallback lineMsgCallback_;  
  TextLineParser lineParser_;
};

}
}

#endif//__FLAGWLD_NET_MSG_TEXTLINECODEC_H_
