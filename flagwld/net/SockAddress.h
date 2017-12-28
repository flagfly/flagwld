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


#ifndef FLAGWLD_NET_SOCKETADDRESS_H
#define FLAGWLD_NET_SOCKETADDRESS_H

#include <flagwld/base/copyable.h>
#include <flagwld/base/StringPiece.h>

#include <netinet/in.h>
#include <sys/un.h>

namespace flagwld
{
namespace net
{
namespace sockets
{
const struct sockaddr* sockaddr_cast(const struct sockaddr_un* addr);
}

/// This is an POD interface class.
class SockAddress : public flagwld::copyable
{
 public:
  explicit SockAddress(uint16_t port=0, bool loopbackOnly = false, bool ipv6=false);
  SockAddress(const StringArg& ip, uint16_t port, bool ipv6=false);

  explicit SockAddress(const char* un_domain);
  SockAddress(const StringArg& un_domain);

  explicit SockAddress(const struct sockaddr* addr);

  explicit SockAddress(const struct sockaddr_in& addr)
    : addr_(addr)
  { }

  explicit SockAddress(const struct sockaddr_in6& addr)
    : addr6_(addr)
  { }

  explicit SockAddress(const struct sockaddr_un& addr)
    : addrun_(addr)
  { }

  sa_family_t family() const { return addr_.sin_family; }

  string toString() const;
  string toShortString() const;

  const struct sockaddr* getSockAddr() const { return sockets::sockaddr_cast(&addrun_); }
  socklen_t getSockLen() const;
  void setSockAddr(const struct sockaddr* addr);

  string toIp() const __attribute__((deprecated)) { return toShortString(); } 
  string toIpPort() const __attribute__((deprecated)) { return toString(); } 

  uint16_t toPort() const;

  uint32_t ipNetEndian() const;
  uint16_t portNetEndian() const;

  // resolve hostname to IP address, not changing port or sin_family
  // return true on success.
  // thread safe
  static bool resolve(StringArg hostname, SockAddress* result);
  // static std::vector<SockAddress> resolveAll(const char* hostname, uint16_t port = 0);

 private:
  union
  {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
    struct sockaddr_un addrun_;
  };
};

}
}

#endif  // FLAGWLD_NET_SOCKETADDRESS_H
