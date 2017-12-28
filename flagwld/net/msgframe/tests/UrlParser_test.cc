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


#include <flagwld/net/msgframe/UrlParser.h>

#include <iostream>

using namespace flagwld;
using namespace flagwld::net;

int main(int argc, char* argv[])
{
  uint16_t port = 80;
  //const char* url = "http://127.0.0.1/hello";
  const char* url = "http://dev.yizhibo.tv/easyvaas/agservice/sched/updatestate";

  UrlParser e;

  if (argc > 1)
  {
    url = argv[1];
  }

  if (!e.Execute(url, strlen(url))) {
    std::cerr << "Error: " << url << std::endl;
    return -1;
  }

  std::cout << e.port() << std::endl;
  std::cout << e.host() << std::endl;
  std::cout << e.proto() << std::endl;
  std::cout << e.path() << std::endl;
  std::cout << e.query() << std::endl;

  const char* url2 = "live://127.0.0.1:1934/fetch?cid=3";
  if (!e.Execute(url2, strlen(url2))) {
    std::cerr << "Error2: " << url2 << std::endl;
    return -1;
  }
  std::cout << e.port() << std::endl;
  std::cout << e.host() << std::endl;
  std::cout << e.proto() << std::endl;
  std::cout << e.path() << std::endl;
  std::cout << e.query() << std::endl;

  return 0;
}

