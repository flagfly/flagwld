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


#ifndef __FLAGWLD_NET_MSG_URLPARSER_H_
#define __FLAGWLD_NET_MSG_URLPARSER_H_

#include <flagwld/base/StringPiece.h>

#include <boost/noncopyable.hpp>

#include <stdint.h>
#include <string>

namespace flagwld
{
namespace net
{

class UrlParser : boost::noncopyable
{
public:
  UrlParser();
  ~UrlParser();

  bool Execute(const char *buf, size_t buflen);

  inline uint16_t port() const { return port_; } 

  inline const string& proto() const { return proto_; } 
  inline const string& host() const { return host_; } 
  inline const string& path() const { return path_; } 
  inline const string& query() const { return query_; } 
  inline const string& fragment() const { return fragment_; } 
  inline const string& usrinfo() const { return usrinfo_; } 

private:
  uint16_t port_;

  string proto_;
  string host_;
  string path_;
  string query_;
  string fragment_;
  string usrinfo_;
};

}
}

#endif//__FLAGWLD_NET_MSG_URLPARSER_H_
