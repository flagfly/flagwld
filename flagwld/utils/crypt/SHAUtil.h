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


#ifndef FLAGWLD_UTILS_SHAUTIL_H
#define FLAGWLD_UTILS_SHAUTIL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <openssl/sha.h>

namespace flagwld
{
namespace utils 
{

bool defaultShaDataCallback(unsigned char*, size_t len);
typedef boost::function<bool (unsigned char*, size_t len)> SHADataCallback;

const size_t kSha1Buffer = SHA_DIGEST_LENGTH;

class SHA1Encoder: boost::noncopyable
{
public:
  explicit SHA1Encoder();
  ~SHA1Encoder();

  inline void setDataCallback(const SHADataCallback& cb)
  { dataCallback_ = cb; }

  int execute(unsigned char*buff, size_t len, bool eof=true);

private:
  SHADataCallback dataCallback_;
  unsigned char buffer_[kSha1Buffer];
  const size_t bufferSize_;
   
  SHA_CTX sha1_;  
};

}
}

#endif  // FLAGWLD_UTILS_SHAUTIL_H
