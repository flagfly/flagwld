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


/*
   Copyright (C) 2010-2016 Yuan Jun <uanjun@hotmail.com>

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
*/

#include <flagwld/utils/base62.h>

#include <sys/types.h>
#include <assert.h>

static const char base62EncodeChars[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

#pragma GCC diagnostic ignored "-Wnarrowing"
static const unsigned char base62DecodeChars[] = {
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	52, 53, 54, 55, 56, 57, 58, 59,
	60, 62, -1, -1, -1, -1, -1, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,
	 7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22,
	23, 24, 25, -1, -1, -1, -1, -1,
	-1, 26, 27, 28, 29, 30, 31, 32,
	33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51, -1, -1, -1, -1, -1,
};
#pragma GCC diagnostic warning "-Wnarrowing"

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
size_t flagwld::utils::base62_encode(unsigned char const *in, size_t inlen, char *out, size_t outlen)
{
        assert (outlen>0);

	register unsigned char bit = 0, n = 0;
	register unsigned char c = 0;
	register size_t i = 0, j = 0;
	while (i < inlen) {
                if (j>=outlen) {
                    return -1;
                }
		if (bit > n + 2) {
			c |= ((in[i] & (0xff >> bit)) << (bit - n - 2));
			n = 8 - bit; bit = 0; ++i;
			continue;
		} else {
			c |= ((in[i] & (0xff >> bit)) >> (n + 2 - bit));
			bit = ((bit + 6 - n) & 7); n = 0;
			if (bit == 0) ++i;
		}
		if ((c & 0x3e) == 0x3e) {
			if (bit > 0) --bit;
			else { bit = 7; --i; }
			out[j] = base62EncodeChars[61];
		} else if ((c & 0x3c) == 0x3c) {
			if (bit > 0) --bit;
			else { bit = 7; --i; }
			out[j] = base62EncodeChars[60];
		} else {
			out[j] = base62EncodeChars[c];
		}
		c = 0; ++j;
	}
	if (n > 0) {
		c >>= (6 - n);
                if ((j+1)>=outlen) return -1;
		out[j++] = base62EncodeChars[c];
	}
	return j;
}

size_t flagwld::utils::base62_decode(char const *in, size_t inlen, unsigned char *out, size_t outlen)
{
        assert (outlen>0);

	register unsigned char bit = 0, n = 0;
	register unsigned char c = 0;
	register size_t i = 0, j = 0;
	while (i + 1 < inlen) {
                if (j>=outlen) return -1;
		if (in[i] < 0) return -1;
		if ((c = base62DecodeChars[(int)in[i]]) == (unsigned char)-1) return -1;
		out[j] &= (0xff << (8 - bit));
		out[j] |= ((c << (2 + n)) >> bit);
		if (bit > 2) {
			n = 8 - bit; bit = 0; ++j;
			continue;
		} else {
			bit = (bit + 6 - n) & 7; n = 0;
			++i; if (bit == 0) ++j;
		}
		if (c == 60 || c == 62) {
			if (bit > 0) --bit;
			else { bit = 7; --j; }
		}
	}
	if (i < inlen) {
		if (in[i] < 0) return -1;
		if ((c = base62DecodeChars[(int)in[i]]) == (unsigned char)-1) return -1;
		if ((c & (0xff >> bit)) != c) return -1;
                if ((j+1)>=outlen) return -1;
		out[j++] |= c;
	}
	return j;
}
#pragma GCC diagnostic error "-Wold-style-cast"
#pragma GCC diagnostic error "-Wconversion"
