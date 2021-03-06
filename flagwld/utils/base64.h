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


#ifndef FLAGWLD_UTILS_BASE64_H
#define FLAGWLD_UTILS_BASE64_H

#include <flagwld/base/Types.h>

#include <sys/types.h>

namespace flagwld
{
namespace utils
{

flagwld::string base64_encode(unsigned char const* bytes_to_encode, size_t in_len);
flagwld::string base64_decode(flagwld::string const& encoded_string);

}
}

#endif //FLAGWLD_UTILS_BASE64_H
