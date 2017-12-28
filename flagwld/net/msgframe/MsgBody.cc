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


#include <flagwld/net/msgframe/MsgBody.h>
#include <flagwld/net/SockConnection.h>

#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS

using namespace flagwld;
using namespace flagwld::net;

MsgBody::MsgBody():type_(KMEMBODY)
{
}

MsgBody::~MsgBody()
{
}

size_t MsgBody::read(char* buf, size_t len)
{
  size_t rlen = body_.readableBytes()<len?body_.readableBytes():len;
  memcpy(buf, body_.peek(), rlen);
  body_.retrieve(rlen);

  return len;
}

void MsgBody::appendChunkToBuffer(Buffer* output, const char* buf, size_t len)
{
  char line[256] = {0};

  int n = snprintf(line, sizeof line, "%"PRIx64"\r\n", len);
  output->append(line, n);
  output->append(buf, len);
  output->append("\r\n", 2);
}

void MsgBody::appendChunkTrailerToBuffer(Buffer* output)
{
  appendChunkToBuffer(output, static_cast<const char*>(0), 0);
  output->append("\r\n", 2);
}

