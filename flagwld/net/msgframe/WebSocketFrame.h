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


#ifndef FLAGWLD_NET_MSG_WEBSOCKETFRAME_H
#define FLAGWLD_NET_MSG_WEBSOCKETFRAME_H

#include <flagwld/net/msgframe/Message.h>

#include <flagwld/base/Timestamp.h>
#include <flagwld/base/Types.h>
#include <flagwld/net/Buffer.h>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>

namespace flagwld
{
namespace net
{

class Buffer;
class SockConnection;
typedef boost::shared_ptr<SockConnection> SockConnectionPtr;

class WebSocketFrame;
typedef boost::shared_ptr<WebSocketFrame> WebSocketFramePtr;

class WebSocketFrame: boost::noncopyable
{
 public:
  enum {
    OPCODE_CONTINUE=0x0,
    OPCODE_TEXT=0x1,
    OPCODE_BINARY=0x2,
    OPCODE_CONNCLOSED=0x8,
    OPCODE_PING=0x9,
    OPCODE_PONG=0xa,
  };

 public:
  WebSocketFrame()
    : fin_(true),
      opcode_(0),
      mask_(true),
      payloadlen_(0),
      timestamp_(Timestamp::now()),
      nmessage_(0),
      nbody_(0),
      nparsed_(0)
  {
  }

  ~WebSocketFrame()
  {
  }

  inline void setFin(bool on)
  { fin_ = on; }

  inline bool fin()
  { return fin_; }

  inline void setOpcode(uint8_t oc)
  { opcode_ = oc; }

  inline uint8_t opcode()
  { return opcode_; }

  inline void setMask(bool on)
  { mask_ = on; }

  inline bool mask()
  { return mask_; }

  void setMaskKey(unsigned char* maskkey, size_t len);

  inline void setTimestamp(Timestamp t)
  { timestamp_ = t; }

  inline Timestamp timestamp() const
  { return timestamp_; }

  void swap(WebSocketFrame& that);

  void reset();

  size_t appendBody( const char* p, size_t len );

  inline size_t nmessage() const { return nmessage_; }
  inline size_t nbody() const { return nbody_; }
  inline size_t nparsed() const { return nparsed_; }

  const char* data()
  { return body_.peek(); }

  size_t length()
  { return body_.readableBytes(); }

  void retrieve(size_t len)
  { body_.retrieve(len); }

  void retrieveAll()
  { body_.retrieveAll(); }

  string asString()
  {
    return string(body_.peek(), body_.readableBytes());
  }

  bool isValid();

  inline void setContext(const boost::any& context)
  { context_ = context; }

  inline const boost::any& getContext() const
  { return context_; }

  inline boost::any* getMutableContext()
  { return &context_; }

  void send(const SockConnectionPtr& conn);

  /// Advanced interface
  inline Buffer* bodyBuffer()
  { return &body_; }

 private:
  friend class WebSocketCodec;

  inline void setnMessage(size_t n) { nmessage_=n; }
  inline void setnBody(size_t n) { nbody_=n; }
  inline void setnParsed(size_t n) { nparsed_=n; }

  size_t appendBodyByParser( const char* p, size_t len );

 private:
  bool fin_;
  uint8_t opcode_;
  bool mask_;
  uint64_t payloadlen_;
  unsigned char maskkey_[4];

  Buffer body_;//unmask data

  Timestamp timestamp_;
  size_t nmessage_;// all message header parsed bytes by parser
  size_t nbody_;//all body parserd bytes by parser
  size_t nparsed_;//all parsed bytes by parser

  boost::any context_;
};

}
}

#endif  // FLAGWLD_NET_MSG_WEBSOCKETFRAME_H
