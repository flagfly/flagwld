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


#ifndef __FLAGWLD_NET_MSG_WEBSOCKETCODEC_H_
#define __FLAGWLD_NET_MSG_WEBSOCKETCODEC_H_

#include <flagwld/net/msgframe/Message.h>
#include <flagwld/net/msgframe/WebSocketFrame.h>
#include <flagwld/net/msgframe/WebSocketFrameParser.h>

#include <flagwld/net/SockConnection.h>

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

#include <sys/types.h>

namespace flagwld
{
namespace net
{

typedef boost::function<void (const SockConnectionPtr&, const WebSocketFramePtr&)> WebSocketFrameCallback;

class WebSocketCodec : boost::noncopyable
{
public:
  WebSocketCodec();
  ~WebSocketCodec();

  void setWebSocketFrameCallback(const WebSocketFrameCallback&cb)
  { webSocketFrameCallback_=cb; }

  void onMessage(const flagwld::net::SockConnectionPtr& conn, flagwld::net::Buffer* buf);

private:
  bool reset();

private:
  inline void setFin(bool on)
  { frame_->setFin(on); }
  inline void setOpcode(uint8_t oc)
  { frame_->setOpcode(oc); }
  inline void setMask(bool on)
  { frame_->setMask(on); }
  inline void setMaskKey(unsigned char* maskkey, size_t len)
  { frame_->setMaskKey(maskkey, len); }
  inline size_t appendBody(const char* p, size_t len)
  { return frame_->appendBodyByParser(p, len); }

private:
  WebSocketFrameCallback webSocketFrameCallback_;
  WebSocketFramePtr frame_;

  WebSocketFrameParser parser_;
};

}
}

#endif//__FLAGWLD_NET_MSG_TEXTLINECODEC_H_
