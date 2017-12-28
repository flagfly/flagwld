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


#include <flagwld/net/msgframe/WebSocketFrame.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/Buffer.h>
#include <flagwld/net/SockConnection.h>

#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS

using namespace flagwld;
using namespace flagwld::net;

static bool isValidUtf8(unsigned char *str, size_t length);

void WebSocketFrame::swap(WebSocketFrame& that)
{
  fin_ = that.fin_;
  opcode_ = that.opcode_;
  mask_ = that.mask_;
  payloadlen_ = that.payloadlen_;
  memset(maskkey_, 0, sizeof(maskkey_));
  timestamp_.swap(that.timestamp_);
  nmessage_=that.nmessage_;
  nbody_=that.nbody_;
  nparsed_=that.nparsed_;
  body_.swap(that.body_);
}

void WebSocketFrame::reset()
{
  fin_ = true;
  opcode_ = 0;
  mask_ = true;
  payloadlen_ = 0;
  memset(maskkey_, 0, sizeof(maskkey_));
  timestamp_ = Timestamp::now();
  nmessage_=0;
  nbody_=0;
  nparsed_=0;
  body_.retrieveAll();
}

void WebSocketFrame::setMaskKey(unsigned char* maskkey, size_t len)
{
  assert (len == 4);

  memcpy(maskkey_, maskkey, len);
}

#pragma GCC diagnostic ignored "-Wold-style-cast"

size_t WebSocketFrame::appendBody( const char* p, size_t len )
{
  size_t l = body_.readableBytes() + len;

  if (l>=DEFAULT_MAX_MEM_BODY_LEN)
  {
    return -1;
  }

  body_.append(p, len);

  return len;
}

size_t WebSocketFrame::appendBodyByParser( const char* p, size_t len )
{
  size_t l = body_.readableBytes() + len;

  if (l>=DEFAULT_MAX_MEM_BODY_LEN)
  {
    return -1;
  }

  if (mask_)
  {
    char c;
    for (size_t i = 0; i < len; ++i) {
      c = (char)(*(p+i) ^ *(maskkey_ + i%4));
      body_.appendInt8(c);
    }
  }
  else
  {
    body_.append(p, len);
  }

  return len;
}

#pragma GCC diagnostic ignored "-Wconversion"
void WebSocketFrame::send(const SockConnectionPtr& conn)
{
//encode
  Buffer buffer;

  unsigned char c1 = 0x00;
  unsigned char c2 = 0x00;

  c1 = c1 | (fin_ << 7);
  c1 = c1 | opcode_;
  c2 = c2 | (mask_ << 7);

  if (body_.readableBytes()==0)
  {
    buffer.appendInt8(c1);
    buffer.appendInt8(c2);
  }
  else if (body_.readableBytes()<=125)
  {
    c2 += body_.readableBytes();

    buffer.appendInt8(c1);
    buffer.appendInt8(c2);
  }
  else if (body_.readableBytes()>=126 && body_.readableBytes()<=65535)
  {
    c2 += 126;

    buffer.appendInt8(c1);
    buffer.appendInt8(c2);
    buffer.appendInt16(body_.readableBytes());
  }
  else if (body_.readableBytes()>=65536)
  {
    c2 += 127;

    buffer.appendInt8(c1);
    buffer.appendInt8(c2);
    buffer.appendInt64(body_.readableBytes());
  }

  if (mask_)
  {
    if (mask_)
    {
      buffer.append(maskkey_, 4);
    }

    const char*p = body_.peek();
    size_t len = body_.readableBytes();

    char c;
    for (size_t i = 0; i < len; ++i) {
      c = (char)(*(p+i) ^ *(maskkey_ + i%4));
      buffer.appendInt8(c);
    }

    conn->send(&buffer);
  }
  else
  {
    buffer.append(body_.peek(), body_.readableBytes());
    conn->send(&buffer);
  }
}

bool WebSocketFrame::isValid()
{
  bool succeed = true;
  switch(opcode_)
  {
    case OPCODE_CONTINUE:
      succeed = fin_;
      break;
    case OPCODE_TEXT:
      succeed = isValidUtf8((unsigned char*)body_.peek(), body_.readableBytes());
      break;
    case OPCODE_BINARY:
      break;
    case OPCODE_CONNCLOSED:
      succeed = fin_;
      break;
    case OPCODE_PING:
      succeed = fin_;
      break;
    case OPCODE_PONG:
      succeed = fin_;
      break;
    default:
      succeed = false;
      break;
  }

  return succeed;
}

static bool isValidUtf8(unsigned char *str, size_t length)
{
    /*
    Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
    OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
    */
    static uint8_t utf8d[] = {
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
      7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
      8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
      0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
      0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
      0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
      1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
      1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
      1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
    };

    // Modified (c) 2016 Alex Hultman
    uint8_t *utf8d_256 = utf8d + 256, state = 0;
    for (int i = 0; i < (int) length; i++) {
        state = utf8d_256[(state << 4) + utf8d[str[i]]];
    }
    return !state;
}

#pragma GCC diagnostic error "-Wconversion"
#pragma GCC diagnostic error "-Wold-style-cast"

