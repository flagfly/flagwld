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


#include <flagwld/utils/crypt/MD5Util.h>
    
#include <flagwld/base/Logging.h>
#include <stdio.h>
    
using namespace flagwld;
using namespace flagwld::utils;
  
bool flagwld::utils::defaultMd5DataCallback(unsigned char*, size_t len)
{ 
  return true;
}

using namespace flagwld;
using namespace flagwld::utils;

MD5Encoder::MD5Encoder()
                :dataCallback_(defaultMd5DataCallback),
                bufferSize_(kMd5Buffer)
{
  MD5_Init(&md5_);
}

MD5Encoder::~MD5Encoder()
{
}

int MD5Encoder::execute(unsigned char*buff, size_t len, bool eof)
{
  int total = 0;

  do
  {
    int in_len = static_cast<int>(len>=bufferSize_?bufferSize_:len);

    if (MD5_Update(&md5_, buff+total, in_len) != 1)
    {
      return -1;
    }

    total += in_len;
    len -= in_len;
  } while(len>0);
  
  if (eof)
  {
    if (MD5_Final(buffer_, &md5_)!=1)
    {
      return -1;
    }

    if (!dataCallback_(buffer_, bufferSize_))
    {
      return -1;
    }
  }

  return total;
}

