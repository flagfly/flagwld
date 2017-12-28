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


#include <flagwld/base/Logging.h>
#include <flagwld/utils/mysql/MysqlUtil.h>

#include <boost/bind.hpp>

using namespace flagwld;
using namespace flagwld::utils;

struct host
{
 string user;
 string host;
};

static bool store(host *p, int i, char const* msg)
{
  switch(i){
    case 0:
       p->user = msg;
       break;
    case 1:
       p->host = msg;
    default:
       return false;
  }

  return true;
}

int main()
{
  MysqlPtr m(new Mysql);

  m->config("127.0.0.1", "root", "10023810");


  if (!m->connect("mysql")) {
    LOG_ERROR << "Connect to mysql://root:10023810@127.0.0.1 " << m->error();
    return -1;
  }

  if (!m->execute("select user,host from user;")) {
    LOG_ERROR << "Execute at mysql://root:10023810@127.0.0.1 " << m->error();
    return -1;
  }

  ResultPtr r = m->result();

  if (!r) {
    LOG_ERROR << "None result at mysql://root:10023810@127.0.0.1";
    return -1;
  }

  int i=0; 
  for(i=0; i<r->countRows(); ++i){
    host h;
    if (!r->storeResult( boost::bind( store, &h, _1, _2) ) ) {
      LOG_ERROR << "Store Error";
      continue;
    }

    LOG_INFO << "id=" << h.user <<"  wan=" << h.host;   
  }

  return 0; 
}
