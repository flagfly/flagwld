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


#include <flagwld/net/msgframe/TextLineCodec.h>

#include <flagwld/base/Logging.h>

using namespace flagwld;
using namespace flagwld::net;

namespace flagwld
{
namespace net
{
namespace detail
{

void defaultLineMsgCallback(const SockConnectionPtr& conn, const char*begin,  size_t len)
{
}

}
}
}

TextLineCodec::TextLineCodec()
  :lineMsgCallback_(detail::defaultLineMsgCallback)  
{
}

TextLineCodec::~TextLineCodec()
{
}

void TextLineCodec::onMessage(const flagwld::net::SockConnectionPtr& conn, flagwld::net::Buffer* buf)
{
  size_t nline=0;

__reexecute_line_byte:
  nline = lineParser_.readLine(buf->peek(), buf->readableBytes());

  if (lineParser_.parserLineError())
  {
    LOG_WARN << "Log line parse error: " << conn->peerAddress().toString();
    conn->shutdown();
    return;
  }

  if (nline == 0)
  {
    return;
  }

  if (nline>0)
  { 
    buf->retrieve(nline);
    nline = 0;
  }

  if (lineParser_.gotLine())
  {
    if (lineMsgCallback_)
    {
      lineMsgCallback_(conn, buf->peek(), lineParser_.nline());
    }

    lineParser_.resetLineState();

    if (buf->readableBytes()>0)
    {
      goto __reexecute_line_byte;
    }
  }
  else
  {
    conn->shutdown();
    return;
  }
}

