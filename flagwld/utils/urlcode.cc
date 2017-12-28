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


#include <flagwld/utils/urlcode.h>

#include <string>

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"

static inline char dec2hexChar(short int n)
{
	if (0 <= n && n <= 9) {
		return char(short('0') + n);
	} else if (10 <= n && n <= 15) {
		return char(short('A') + n - 10);
	}
	return char(0);
}

static inline short int hexChar2dec(char c)
{
	if ('0'<=c && c<='9') {
		return short(c-'0');
	} else if ('a'<=c && c<='f') {
		return (short(c-'a') + 10);
	} else if ('A'<=c && c<='F') {
		return (short(c-'A') + 10);
	}
	return -1;
}

flagwld::string flagwld::utils::url_encode(flagwld::string const &URL)
{
	flagwld::string result = "";
	for (unsigned int i=0; i<URL.length(); i++) {
		char c = URL[i];
		if (('0'<=c && c<='9') ||
			('a'<=c && c<='z') ||
			('A'<=c && c<='Z') ||
			c=='/' || c=='.') {
			result += c;
		} else {
			int j = (short int)c;
			if (j < 0) {
				j += 256;
			}
			int i1, i0;
			i1 = j / 16;
			i0 = j - i1*16;
			result += '%';
			result += dec2hexChar(i1);
			result += dec2hexChar(i0);
		}
	}
	return result;
}

flagwld::string flagwld::utils::url_decode(flagwld::string const &URL)
{
	flagwld::string result = "";
	for (unsigned int i=0; i<URL.length(); i++) {
		char c = URL[i];
		if (c != '%') {
			result += c;
		} else {
			char c1 = URL[++i];
			char c0 = URL[++i];
			int num = 0;
			num += hexChar2dec(c1) * 16 + hexChar2dec(c0);
			result += char(num);
		}
	}
	return result;
}

#pragma GCC diagnostic error "-Wold-style-cast"
#pragma GCC diagnostic error "-Wconversion"
