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


#include <flagwld/net/msgframe/WebSocketCodec.h>
#include <flagwld/net/msgframe/WebSocketFrame.h>
#include <flagwld/net/msgframe/WebSocketFrameParser.h>

#include <flagwld/base/Logging.h>

#include <boost/bind.hpp>

using namespace flagwld;
using namespace flagwld::net;

namespace flagwld
{
namespace net
{
namespace detail
{

void defaultWebSocketFrameCallback(const SockConnectionPtr& conn, const WebSocketFramePtr& frame)
{
  switch (frame->opcode())
  {
    case WebSocketFrame::OPCODE_CONTINUE:
      LOG_TRACE << "recv OPCODE_CONTINUE len:" << frame->length();
      break;
    case WebSocketFrame::OPCODE_TEXT:
      LOG_TRACE << "recv OPCODE_TEXT len:" << frame->length();
      break;
    case WebSocketFrame::OPCODE_BINARY:
      LOG_TRACE << "recv OPCODE_BINARY len:" << frame->length();
      break;
    case WebSocketFrame::OPCODE_CONNCLOSED:
      LOG_TRACE << "recv OPCODE_CONNCLOSED len:" << frame->length();
      conn->shutdown();
      break;
    case WebSocketFrame::OPCODE_PING:
      LOG_TRACE << "recv OPCODE_PING len:" << frame->length();
      frame->retrieveAll();
      frame->setFin(true);
      frame->setOpcode(WebSocketFrame::OPCODE_PONG);
      frame->send(conn);
      break;
    case WebSocketFrame::OPCODE_PONG:
      LOG_TRACE << "recv OPCODE_PONG len:" << frame->length();
      break;
    default:
      LOG_TRACE << "recv OPCODE_UNKNOW len:" << frame->length();
      break;
  }
}

}
}
}

/****************************************** WebSocketClientCodec ******************************************/

WebSocketCodec::WebSocketCodec()
  :webSocketFrameCallback_(detail::defaultWebSocketFrameCallback),
  frame_(new WebSocketFrame)
{
  parser_.setFinCallback( boost::bind(&WebSocketCodec::setFin, this, _1) );
  parser_.setOpcodeCallback( boost::bind(&WebSocketCodec::setOpcode, this, _1) );
  parser_.setMaskCallback( boost::bind(&WebSocketCodec::setMask, this, _1) );
  parser_.setMaskKeyCallback( boost::bind(&WebSocketCodec::setMaskKey, this, _1, _2) );
  parser_.setBodyCallback( boost::bind(&WebSocketCodec::appendBody, this, _1, _2) );
}

WebSocketCodec::~WebSocketCodec()
{
}

bool WebSocketCodec::reset()
{
  parser_.reset();
  frame_.reset(new WebSocketFrame);

  return frame_;
}

void WebSocketCodec::onMessage(const flagwld::net::SockConnectionPtr& conn, flagwld::net::Buffer* buf)
{
  size_t nparsed = 0;

__reexecute_byte:
  nparsed = parser_.parseExecute( buf->peek(), buf->readableBytes() );
  LOG_TRACE << "["<<buf->readableBytes()<<"] nparsed=" << nparsed;

  if (parser_.parseError())
  {
    conn->shutdown();
    conn->stopRead();
    return ;
  }

  if (nparsed == 0)
  {
    return;
  }

  if (nparsed > 0)
  {
    buf->retrieve(nparsed);
    nparsed = 0;
  }

  if (parser_.gotFrame())
  {
    if (!frame_->isValid())
    {
      conn->shutdown();
      conn->stopRead();
      return ; 
    }

    frame_->setnMessage(parser_.nmessage());
    frame_->setnBody(parser_.nbody());
    frame_->setnParsed(parser_.nread());
    frame_->setTimestamp(Timestamp::now());

    webSocketFrameCallback_(conn, frame_);

    if (reset())
    {
      if (buf->readableBytes()>0)
      {
        goto __reexecute_byte;
      }
    }
    else
    {
      conn->shutdown();
      conn->stopRead();
    }
  }
}

