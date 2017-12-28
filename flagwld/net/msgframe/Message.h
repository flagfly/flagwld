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


#ifndef FLAGWLD_NET_MSG_MESSAGE_H
#define FLAGWLD_NET_MSG_MESSAGE_H

namespace flagwld
{
namespace net
{

enum ParseMsgState
{
  ParseMsgBeginE=0,
  ParseMsgHeaderE,
  ParseMsgBodyE,
  ParseMsgFinalE
};

#define MAX_LINE_LEN 2048
#define MAX_MESSAGE_LEN 80*1024
#define DEFAULT_MAX_MEM_BODY_LEN 8*1024*1024

}
}
#endif //FLAGWLD_NET_MSG_MESSAGE_H
