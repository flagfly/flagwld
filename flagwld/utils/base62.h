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


#ifndef FLAGWLD_UTILS_BASE62_H
#define FLAGWLD_UTILS_BASE62_H

#include <sys/types.h>

namespace flagwld
{
namespace utils
{

size_t base62_encode(unsigned char const *in, size_t inlen, char *out, size_t outlen);
size_t base62_decode(char const *in, size_t inlen, unsigned char *out, size_t outlen);

#define base62_encoded_length(len)  ((len * 4 + 2) / 3)
#define base62_decoded_length(len)  (((len + 3) / 4) * 3)

}
}

#endif //FLAGWLD_UTILS_BASE62_H
