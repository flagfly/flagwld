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


#ifndef __FLAGWLD_NET_MSG_WEBSOCKETFRAMEPARSER_H_
#define __FLAGWLD_NET_MSG_WEBSOCKETFRAMEPARSER_H_

#include <flagwld/net/msgframe/Message.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <sys/types.h>

namespace flagwld
{
namespace net
{

class WebSocketFrameParser : boost::noncopyable
{
public:
  enum WSFrameState
  {
    WSFrameErrorE=-1,
    WSFrameHead2BytesE,
    WSFramePayloadLenMore2BytesE,
    WSFramePayloadLenMore8BytesE,
    WSFrameMaskey4BytesE,
    WSFrameDataE,
    WSFrameEndE,
  };

public:
  typedef boost::function<void(bool)> FinCallback;
  typedef boost::function<void(uint8_t)> OpcodeCallback;
  typedef boost::function<void(bool)> MaskCallback;
  typedef boost::function<void(unsigned char* maskkey, size_t)> MaskKeyCallback;  
  typedef boost::function<size_t(const char*, size_t)> BodyCallback;

public:
  WebSocketFrameParser();
  ~WebSocketFrameParser();

  void setFinCallback(const FinCallback& cb) { finCallback_ = cb; }
  void setOpcodeCallback(const OpcodeCallback& cb) { opcodeCallback_ = cb; }
  void setMaskCallback(const MaskCallback& cb) { maskCallback_ = cb; }
  void setMaskKeyCallback(const MaskKeyCallback& cb) { maskkeyCallback_ = cb; }
  void setBodyCallback(const BodyCallback& cb) { bodyCallback_ = cb; }

  size_t parseExecute(const char*data, size_t len);

  inline bool gotFrame(){ return (wsstate_==WSFrameEndE); }

  inline size_t nmessage() { return nmessage_; }
  inline size_t nbody() { return nbody_; }
  inline size_t nread() { return nread_; }

  inline bool parseError() { return (wsstate_==WSFrameErrorE); }
  void reset();

private:
  bool processHead2Bytes(const char* begin, size_t len);
  bool processPayloadLenMore2Bytes(const char* begin, size_t len);
  bool processPayloadLenMore8Bytes(const char* begin, size_t len);
  bool processMasyKey4Bytes(const char* begin, size_t len);
  bool processBody(const char* begin, size_t len);

private:
  FinCallback finCallback_;
  OpcodeCallback opcodeCallback_;
  MaskCallback maskCallback_;
  MaskKeyCallback maskkeyCallback_;
  BodyCallback bodyCallback_;

  WSFrameState wsstate_;

  bool mask_;

  size_t nmessage_;// all message header parsed bytes by parser
  size_t nbody_;//all body parserd bytes by parser
  size_t bindex_;

  size_t nread_;//all parsed bytes by parser

};

}
}

#endif//__FLAGWLD_NET_MSG_WEBSOCKETFRAMEPARSER_H_
