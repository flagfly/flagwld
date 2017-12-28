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


#include <sys/types.h>
#include <stdint.h>

uint32_t BKDRHash(void const *str, size_t len)
{
	register unsigned char *p = (unsigned char*)str;
	register uint32_t hash = 0;
	register uint32_t seed = 131;  //  31 131 1313 13131 131313 etc..
	while (len-- > 0) {
		hash = hash * seed + (*p++);
	}
	return hash;
}
