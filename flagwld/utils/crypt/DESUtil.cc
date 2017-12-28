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


#include <flagwld/utils/crypt/DESUtil.h>
    
#include <flagwld/base/Logging.h>
    
using namespace flagwld;
using namespace flagwld::utils;
  
bool flagwld::utils::defaultDesDataCallback(unsigned char*, size_t len)
{ 
  return true;
}

using namespace flagwld;
using namespace flagwld::utils;

DES3Encoder::DES3Encoder(const char*k1,
                         const char*k2,
                         const char*k3,
                         const char*ivec)
               :dataCallback_(defaultDesDataCallback),
                bufferSize_(kDesBuffer)
{
  DES_cblock kb;

  DES_string_to_key(k1, &kb);
  DES_set_key_unchecked(&kb, &ks1_);

  DES_string_to_key(k2, &kb);
  DES_set_key_unchecked(&kb, &ks2_);

  DES_string_to_key(k3, &kb);
  DES_set_key_unchecked(&kb, &ks3_);

  DES_string_to_key(ivec, &ivec_);
}

DES3Encoder::~DES3Encoder()
{
}

#pragma GCC diagnostic ignored "-Wsign-compare"
int DES3Encoder::execute(unsigned char*buff, size_t len)
{
  int total=0;

  do
  {
    int in_len = static_cast<int>(len>=bufferSize_?bufferSize_:len);
    
    int padding = in_len;

    int mod = in_len%8;
    if (mod)
    {
      padding = in_len + (8-mod);
    }

    DES_ede3_cbc_encrypt(buff+total, buffer_, in_len, &ks1_, &ks2_, &ks3_, &ivec_, DES_ENCRYPT);

    if (!dataCallback_(buffer_, padding))
    {
      return -1;
    }

    total += in_len;
    len -= in_len;
  } while(len>0);

  return total;
}
#pragma GCC diagnostic warning "-Wsign-compare"

DES3Decoder::DES3Decoder(const char*k1,
                         const char*k2,
                         const char*k3,
                         const char*ivec)
               :dataCallback_(defaultDesDataCallback),
                bufferSize_(kDesBuffer)
{
  DES_cblock kb;

  DES_string_to_key(k1, &kb);
  DES_set_key_unchecked(&kb, &ks1_);

  DES_string_to_key(k2, &kb);
  DES_set_key_unchecked(&kb, &ks2_);

  DES_string_to_key(k3, &kb);
  DES_set_key_unchecked(&kb, &ks3_);

  DES_string_to_key(ivec, &ivec_);
}

DES3Decoder::~DES3Decoder()
{
}

#pragma GCC diagnostic ignored "-Wsign-compare"
int DES3Decoder::execute(unsigned char*buff, size_t len)
{
  int total=0;

  do
  {
    int in_len = static_cast<int>(len>=bufferSize_?bufferSize_:len);

    if (! in_len%8)
    {
      return -1;
    }

    DES_ede3_cbc_encrypt(buff+total, buffer_, in_len, &ks1_, &ks2_, &ks3_, &ivec_, DES_DECRYPT);

    if (!dataCallback_(buffer_, in_len))
    {
      return -1;
    }

    total += in_len;
    len -= in_len;
  } while(len>0);

  return total;
}
#pragma GCC diagnostic warning "-Wsign-compare"

