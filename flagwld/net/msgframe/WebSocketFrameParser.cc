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


#include <flagwld/net/msgframe/WebSocketFrameParser.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/Endian.h>

using namespace flagwld;
using namespace flagwld::net;

WebSocketFrameParser::WebSocketFrameParser():
                 wsstate_(WSFrameHead2BytesE),
                 mask_(false),
                 nmessage_(0),
                 nbody_(0),
                 nread_(0)
{
}

WebSocketFrameParser::~WebSocketFrameParser()
{
}

/*
0: need parse more
>0: got line, return line length
*/
size_t WebSocketFrameParser::parseExecute(const char*data, size_t len)
{
  size_t nparsed=0;

__reexecute_byte:
  switch (wsstate_)
  {
    case WSFrameHead2BytesE:
      if ((len-nparsed)>=2)
      {
        if (!processHead2Bytes(data+nparsed, 2)) {
              LOG_DEBUG << "Process Head 2 Bytes, Error";
              wsstate_=WSFrameErrorE;
              goto __parse_done;
        }

        nparsed += 2;
        nread_ += 2;

        if (nbody_< 126)
        {
          if (mask_)
          {
            wsstate_ = WSFrameMaskey4BytesE;
          }
          else
          {
            wsstate_ = WSFrameDataE;
            nmessage_ = nread_;
            bindex_ = 0;
          }
        }
        else if (nbody_==126)
        {
          wsstate_ = WSFramePayloadLenMore2BytesE;

        }
        else if (nbody_==127)
        {
          wsstate_ = WSFramePayloadLenMore8BytesE;
        }
      }

      if (nparsed<=len)
      {
        goto __reexecute_byte;
      }

      break;

    case WSFramePayloadLenMore2BytesE:
      if ((len-nparsed)>=2)
      {
        if (!processPayloadLenMore2Bytes(data+nparsed, 2)) {
              LOG_DEBUG << "Process Payload Len more 2 Bytes, Error";
              wsstate_=WSFrameErrorE;
              goto __parse_done;
        }

        nparsed += 2;
        nread_ += 2;

        if (mask_)
        {
          wsstate_ = WSFrameMaskey4BytesE;
        }
        else
        {
          wsstate_ = WSFrameDataE;
          nmessage_ = nread_;
          bindex_ = 0;
        }
      }

      if (nparsed<=len)
      {
        goto __reexecute_byte;
      }

      break;

    case WSFramePayloadLenMore8BytesE:
      if ((len-nparsed)>=8)
      {
        if (!processPayloadLenMore8Bytes(data+nparsed, 8)) {
              LOG_DEBUG << "Process Payload Len more 8 Bytes, Error";
              wsstate_=WSFrameErrorE;
              goto __parse_done;
        }

        nparsed += 8;
        nread_ += 2;

        if (mask_)
        {
          wsstate_ = WSFrameMaskey4BytesE;
        }
        else
        {
          wsstate_ = WSFrameDataE;
          nmessage_ = nread_;
          bindex_ = 0;
        }
      }

      if (nparsed<=len)
      {
        goto __reexecute_byte;
      }

      break;

    case WSFrameMaskey4BytesE:
      if ((len-nparsed)>=4)
      {
        if (!processMasyKey4Bytes(data+nparsed, 4)) {
              LOG_DEBUG << "Process Payload Len more 8 Bytes, Error";
              wsstate_=WSFrameErrorE;
              goto __parse_done;
        }

        nparsed += 4;
        nread_ += 2;

        wsstate_ = WSFrameDataE;
        nmessage_ = nread_;
        bindex_ = 0;
      }

      if (nparsed<=len)
      {
        goto __reexecute_byte;
      }
      break;

    case WSFrameDataE:
    {
      size_t l = nbody_-bindex_;
      size_t left = len-nparsed;
      l = l<left?l:left;
      if (l>0)
      {
        if (!processBody(data+nparsed, l) )
        {
          LOG_DEBUG << "Process Msg Body , Error";
          wsstate_=WSFrameErrorE;
          goto __parse_done;
        }
        nparsed += l;
        bindex_ += l;
        nread_ += l;
      }

      if (bindex_==nbody_){
        wsstate_=WSFrameEndE;
        goto __reexecute_byte;
      }
    }
      break;

    case WSFrameEndE:
      LOG_TRACE << "Frame Received: Total:" << nread_ << " Header:" << nmessage_ << " Body:" << nbody_;
      goto __parse_done;
      break;

    default:
      LOG_ERROR ;
      wsstate_ = WSFrameErrorE;
      goto __parse_done;
      break;
  }

__parse_done:
  return nparsed;
}

void WebSocketFrameParser::reset()
{
  wsstate_=WSFrameHead2BytesE;
  mask_ = true;
  nmessage_=0;
  nbody_=0;
  nread_=0;
}

#pragma GCC diagnostic ignored "-Wconversion"
bool WebSocketFrameParser::processHead2Bytes(const char* begin, size_t len)
{
  assert (len == 2);

  unsigned char c1 = *begin;
  unsigned char c2 = *(begin+ 1);

  uint8_t fin = ((c1 >> 7) & 0xff);
  uint8_t opcode = (c1 & 0x0f);
  uint8_t mask = (c2 >> 7) & 0xff;

  nbody_ = c2 & 0x7f;

  if (!(fin<=1 && opcode<=0xf && mask<=1))
  {
    return false;
  }

  finCallback_( fin==1 );

  opcodeCallback_( opcode );

  mask_ = (mask==1);
  maskCallback_(mask_);

  LOG_TRACE << "fin=" << fin << " opcode=" << opcode << " mask=" << mask_ << " nobdy=" << nbody_;

  return true;
}
#pragma GCC diagnostic error "-Wconversion"

#pragma GCC diagnostic ignored "-Wold-style-cast"
bool WebSocketFrameParser::processPayloadLenMore2Bytes(const char* begin, size_t len)
{
  assert (len == 2);

  nbody_ = sockets::networkToHost16( *((uint16_t*)begin) );

  return true;
}

bool WebSocketFrameParser::processPayloadLenMore8Bytes(const char* begin, size_t len)
{
  assert (len == 8);

  nbody_ = sockets::networkToHost64( *((uint64_t*)begin) );

  return true;
}

bool WebSocketFrameParser::processMasyKey4Bytes(const char* begin, size_t len)
{
  assert (len == 4);

  maskkeyCallback_((unsigned char*)begin, len);
  
  return true;
}
#pragma GCC diagnostic error "-Wold-style-cast"

bool WebSocketFrameParser::processBody(const char* begin, size_t len)
{
  assert (bindex_ <= nbody_);

  bool succeed = true;
  if (bodyCallback_(begin, len) != len)
  {
    succeed = false;
  }
  return succeed;
}

