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


/*-------------------------------------------------------------------------
 integer to string conversion

 Written by:   Bela Torok, 1999
               bela.torok@kssg.ch
 usage:

 uitoa(unsigned int value, char* string, int radix)
 itoa(int value, char* string, int radix)

 value  ->  Number to be converted
 string ->  Result
 radix  ->  Base of value (e.g.: 2 for binary, 10 for decimal, 16 for hex)
---------------------------------------------------------------------------*/

#define NUMBER_OF_DIGITS 16   /* space for NUMBER_OF_DIGITS + '\0' */
/*
void uitoa(unsigned int value, char *string, int radix)
{
	unsigned char index, i;

	index = NUMBER_OF_DIGITS;
	i = 0;

	do {
		string[--index] = '0' + (value % radix);
		if (string[index] > '9') string[index] += 'A' - ':';   // continue with A, B,..
		value /= radix;
	} while (value != 0);

	do {
		string[i++] = string[index++];
	} while (index < NUMBER_OF_DIGITS);

	string[i] = 0; // string terminator
}

void itoa(int value, char *string, int radix)
{
	if (value < 0 && radix == 10) {
		*string++ = '-';
		uitoa(-value, string, radix);
	} else {
		uitoa(value, string, radix);
	}
}
*/
#include <string>

std::string num_to_str(long long num)
{
	std::string numstr;
	if (num < 0) {
		num = - num;
		numstr = '-';
	}
	if (num > 0) {
		long long quot = num / 10;
		if (quot > 0) numstr += num_to_str(quot);
		numstr += '0' + (num % 10);
	} else {
		numstr += '0';
	}
	return numstr;
}
