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


#include <flagwld/utils/crypt/XXTEAUtil.h>
#include <flagwld/utils/crypt/xxtea/xxtea.h>
    
#include <flagwld/base/Logging.h>
#include <flagwld/base/ScopeGuard.h>

#include <boost/bind.hpp>

using namespace flagwld;
using namespace flagwld::utils;
  
bool flagwld::utils::defaultXXTeaDataCallback(unsigned char*, size_t len)
{ 
  return true;
}

using namespace flagwld;
using namespace flagwld::utils;

XXTEAEncoder::XXTEAEncoder(const char*k)
               :dataCallback_(defaultXXTeaDataCallback),
                key_(k)
{
}

XXTEAEncoder::~XXTEAEncoder()
{
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
int XXTEAEncoder::execute(unsigned char*buff, size_t len)
{
  size_t encryptLen = 0;

  unsigned char* encryptData = (unsigned char*)xxtea_encrypt(buff, len, key_.c_str(), &encryptLen);

  if (!encryptData)
  {
    return -1;
  }

  ScopeGuard buffguard(boost::bind(free, encryptData));

  if (!dataCallback_(encryptData, encryptLen))
  {
    return -1;
  }

  return (int)encryptLen;
}
#pragma GCC diagnostic error "-Wold-style-cast"

XXTEADecoder::XXTEADecoder(const char*k)
               :dataCallback_(defaultXXTeaDataCallback),
                key_(k)
{
}

XXTEADecoder::~XXTEADecoder()
{
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
int XXTEADecoder::execute(unsigned char*buff, size_t len)
{
  size_t decryptLen = 0;

  unsigned char* decryptData = (unsigned char*)xxtea_decrypt(buff, len, key_.c_str(), &decryptLen);

  if (!decryptData)
  {
    return -1;
  }

  ScopeGuard buffguard(boost::bind(free, decryptData));

  if (!dataCallback_(decryptData, decryptLen))
  {
    return -1;
  }

  return (int)decryptLen;
}
#pragma GCC diagnostic error "-Wold-style-cast"
