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


#include <flagwld/utils/crypt/RSAUtil.h>

#include <boost/bind.hpp>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace flagwld;
using namespace flagwld::utils;

unsigned short rsakey[] = {
0x2d,0x2d,0x2d,0x2d,0x2d,0x42,0x45,0x47,0x49,0x4e,0x20,0x52,0x53,0x41,0x20,0x50,
0x52,0x49,0x56,0x41,0x54,0x45,0x20,0x4b,0x45,0x59,0x2d,0x2d,0x2d,0x2d,0x2d,0x0a,
0x4d,0x49,0x49,0x45,0x6f,0x77,0x49,0x42,0x41,0x41,0x4b,0x43,0x41,0x51,0x45,0x41,
0x74,0x68,0x70,0x41,0x45,0x78,0x4b,0x57,0x4e,0x49,0x4c,0x34,0x49,0x2f,0x49,0x35,
0x6e,0x46,0x66,0x41,0x57,0x55,0x73,0x43,0x48,0x34,0x54,0x44,0x46,0x47,0x68,0x62,
0x76,0x4e,0x65,0x4f,0x77,0x72,0x38,0x43,0x73,0x57,0x51,0x4d,0x6a,0x69,0x31,0x42,
0x0a,0x5a,0x49,0x46,0x71,0x42,0x43,0x77,0x69,0x75,0x41,0x51,0x7a,0x36,0x50,0x36,
0x32,0x55,0x35,0x71,0x61,0x5a,0x5a,0x79,0x71,0x4d,0x30,0x30,0x5a,0x45,0x33,0x4d,
0x5a,0x38,0x63,0x31,0x6b,0x74,0x66,0x76,0x4c,0x78,0x4a,0x68,0x37,0x62,0x34,0x69,
0x62,0x37,0x36,0x52,0x6c,0x48,0x66,0x72,0x78,0x6d,0x54,0x44,0x4b,0x66,0x79,0x6b,
0x37,0x0a,0x79,0x30,0x75,0x4c,0x34,0x2f,0x55,0x57,0x53,0x44,0x52,0x31,0x50,0x4d,
0x65,0x59,0x4c,0x4c,0x65,0x6e,0x42,0x47,0x4f,0x64,0x53,0x4a,0x59,0x78,0x6d,0x7a,
0x44,0x79,0x44,0x70,0x76,0x34,0x6a,0x41,0x44,0x33,0x30,0x36,0x41,0x58,0x6d,0x30,
0x64,0x73,0x66,0x74,0x74,0x71,0x55,0x45,0x6a,0x58,0x6f,0x4b,0x6d,0x68,0x36,0x4b,
0x53,0x48,0x0a,0x69,0x70,0x56,0x31,0x4d,0x45,0x66,0x55,0x66,0x2b,0x56,0x34,0x6c,
0x47,0x62,0x58,0x79,0x4e,0x39,0x72,0x53,0x6e,0x4e,0x36,0x4e,0x37,0x6f,0x4f,0x7a,
0x37,0x6c,0x6f,0x48,0x2b,0x47,0x55,0x4f,0x6d,0x4d,0x55,0x49,0x55,0x54,0x5a,0x70,
0x63,0x56,0x38,0x51,0x77,0x2b,0x30,0x4e,0x46,0x44,0x69,0x75,0x66,0x66,0x66,0x75,
0x79,0x71,0x6e,0x0a,0x64,0x70,0x74,0x49,0x4c,0x66,0x6c,0x59,0x70,0x41,0x4f,0x47,
0x74,0x76,0x6b,0x49,0x44,0x42,0x54,0x37,0x51,0x6d,0x63,0x66,0x77,0x2b,0x4d,0x69,
0x30,0x57,0x70,0x76,0x68,0x43,0x63,0x77,0x6b,0x2b,0x34,0x4a,0x79,0x64,0x75,0x32,
0x39,0x51,0x30,0x6d,0x41,0x57,0x36,0x45,0x6f,0x30,0x51,0x31,0x75,0x79,0x30,0x4c,
0x71,0x57,0x2f,0x64,0x0a,0x6c,0x61,0x74,0x69,0x44,0x57,0x66,0x2f,0x38,0x53,0x51,
0x61,0x49,0x77,0x45,0x42,0x54,0x46,0x43,0x63,0x51,0x69,0x52,0x56,0x32,0x67,0x79,
0x42,0x77,0x52,0x57,0x67,0x67,0x31,0x44,0x77,0x6f,0x77,0x49,0x44,0x41,0x51,0x41,
0x42,0x41,0x6f,0x49,0x42,0x41,0x43,0x76,0x74,0x58,0x79,0x46,0x36,0x72,0x56,0x4d,
0x54,0x65,0x4d,0x6f,0x72,0x0a,0x42,0x79,0x79,0x6f,0x6d,0x6c,0x53,0x76,0x33,0x46,
0x35,0x41,0x42,0x31,0x4c,0x6e,0x37,0x61,0x45,0x37,0x47,0x6b,0x76,0x75,0x43,0x62,
0x64,0x43,0x33,0x43,0x68,0x37,0x30,0x44,0x64,0x76,0x5a,0x47,0x53,0x48,0x52,0x51,
0x54,0x41,0x74,0x73,0x6a,0x77,0x6b,0x49,0x6a,0x51,0x56,0x59,0x68,0x48,0x5a,0x32,
0x4d,0x69,0x71,0x50,0x62,0x68,0x0a,0x49,0x42,0x51,0x4e,0x4b,0x4d,0x73,0x68,0x5a,
0x30,0x45,0x51,0x53,0x30,0x35,0x58,0x56,0x4d,0x35,0x56,0x43,0x4c,0x4e,0x58,0x73,
0x6d,0x6e,0x79,0x74,0x42,0x5a,0x43,0x6d,0x78,0x4c,0x71,0x4d,0x56,0x2b,0x57,0x34,
0x6e,0x34,0x65,0x63,0x56,0x31,0x74,0x42,0x45,0x39,0x64,0x4b,0x65,0x50,0x4d,0x4a,
0x38,0x65,0x33,0x75,0x31,0x57,0x6c,0x0a,0x55,0x2b,0x36,0x6b,0x50,0x79,0x74,0x2b,
0x6e,0x45,0x6d,0x6e,0x5a,0x36,0x70,0x6b,0x61,0x30,0x36,0x61,0x6e,0x53,0x57,0x71,
0x76,0x4b,0x45,0x62,0x67,0x68,0x31,0x63,0x6e,0x58,0x6e,0x71,0x36,0x75,0x74,0x71,
0x49,0x55,0x77,0x7a,0x47,0x4e,0x76,0x51,0x34,0x38,0x72,0x2b,0x39,0x55,0x35,0x4f,
0x70,0x74,0x4d,0x6b,0x32,0x53,0x6a,0x35,0x0a,0x34,0x6d,0x57,0x6c,0x74,0x55,0x31,
0x6e,0x6e,0x2f,0x54,0x35,0x6c,0x75,0x30,0x50,0x6d,0x34,0x62,0x7a,0x4c,0x4b,0x76,
0x70,0x33,0x48,0x39,0x75,0x4d,0x48,0x6b,0x5a,0x6f,0x47,0x48,0x69,0x49,0x56,0x71,
0x2f,0x51,0x69,0x34,0x4f,0x49,0x50,0x66,0x49,0x35,0x74,0x2b,0x6d,0x4c,0x43,0x72,
0x4a,0x59,0x58,0x6f,0x73,0x30,0x32,0x59,0x32,0x0a,0x39,0x41,0x4f,0x67,0x47,0x42,
0x46,0x6e,0x34,0x47,0x63,0x65,0x4c,0x56,0x52,0x59,0x46,0x41,0x47,0x36,0x50,0x61,
0x45,0x6e,0x78,0x55,0x4d,0x56,0x37,0x4b,0x53,0x6e,0x55,0x6c,0x71,0x59,0x37,0x54,
0x67,0x2b,0x41,0x44,0x7a,0x76,0x48,0x71,0x4b,0x45,0x50,0x68,0x45,0x43,0x6b,0x53,
0x56,0x67,0x57,0x67,0x41,0x54,0x53,0x55,0x35,0x61,0x0a,0x79,0x65,0x51,0x5a,0x36,
0x41,0x45,0x43,0x67,0x59,0x45,0x41,0x34,0x47,0x46,0x31,0x59,0x6b,0x48,0x54,0x50,
0x36,0x79,0x46,0x4b,0x58,0x46,0x51,0x36,0x69,0x53,0x2b,0x59,0x4c,0x6b,0x6b,0x44,
0x71,0x7a,0x58,0x78,0x69,0x67,0x79,0x78,0x6c,0x38,0x6b,0x66,0x4c,0x54,0x4b,0x65,
0x39,0x72,0x50,0x51,0x58,0x6f,0x6b,0x78,0x57,0x2b,0x55,0x0a,0x51,0x56,0x59,0x79,
0x50,0x79,0x59,0x69,0x57,0x70,0x67,0x54,0x51,0x79,0x51,0x5a,0x36,0x35,0x70,0x33,
0x32,0x66,0x45,0x52,0x59,0x44,0x4a,0x46,0x2f,0x44,0x66,0x39,0x39,0x4f,0x39,0x78,
0x42,0x39,0x66,0x65,0x30,0x37,0x4b,0x7a,0x51,0x57,0x4d,0x4a,0x51,0x4b,0x6d,0x53,
0x4d,0x6e,0x75,0x57,0x6a,0x58,0x51,0x4c,0x4f,0x49,0x62,0x69,0x0a,0x43,0x4f,0x44,
0x43,0x79,0x34,0x6d,0x32,0x34,0x33,0x73,0x76,0x36,0x52,0x4d,0x63,0x49,0x73,0x45,
0x6f,0x4d,0x4d,0x37,0x4a,0x6b,0x4a,0x55,0x34,0x4b,0x46,0x32,0x74,0x6d,0x58,0x41,
0x2b,0x46,0x74,0x2b,0x41,0x65,0x70,0x43,0x6a,0x52,0x69,0x45,0x73,0x4c,0x52,0x72,
0x61,0x4e,0x41,0x45,0x43,0x67,0x59,0x45,0x41,0x7a,0x38,0x4f,0x62,0x0a,0x45,0x35,
0x2f,0x34,0x4b,0x76,0x4f,0x47,0x6e,0x4b,0x4f,0x36,0x2f,0x35,0x53,0x49,0x78,0x2f,
0x63,0x4f,0x71,0x2b,0x54,0x50,0x6c,0x61,0x63,0x6d,0x31,0x4b,0x6b,0x55,0x56,0x72,
0x54,0x68,0x70,0x46,0x53,0x4e,0x44,0x46,0x63,0x43,0x46,0x79,0x35,0x44,0x34,0x42,
0x61,0x45,0x34,0x73,0x2f,0x74,0x45,0x6c,0x36,0x30,0x4e,0x69,0x46,0x65,0x0a,0x37,
0x47,0x70,0x57,0x2b,0x46,0x59,0x67,0x47,0x51,0x45,0x44,0x68,0x7a,0x47,0x6f,0x36,
0x6c,0x51,0x46,0x55,0x76,0x31,0x64,0x54,0x4c,0x53,0x7a,0x76,0x2b,0x54,0x6f,0x72,
0x31,0x61,0x65,0x2b,0x50,0x78,0x78,0x43,0x72,0x54,0x30,0x2b,0x46,0x75,0x46,0x39,
0x2f,0x2f,0x67,0x46,0x68,0x2b,0x37,0x50,0x52,0x36,0x35,0x69,0x45,0x33,0x67,0x0a,
0x39,0x37,0x77,0x47,0x44,0x61,0x35,0x75,0x47,0x7a,0x78,0x6a,0x4d,0x75,0x77,0x67,
0x67,0x43,0x32,0x4c,0x2f,0x41,0x6a,0x62,0x4e,0x79,0x54,0x38,0x32,0x45,0x7a,0x38,
0x42,0x55,0x4e,0x52,0x31,0x4b,0x4d,0x43,0x67,0x59,0x42,0x30,0x53,0x79,0x37,0x61,
0x75,0x55,0x45,0x79,0x2f,0x45,0x69,0x63,0x48,0x53,0x59,0x52,0x35,0x4f,0x39,0x6a,
0x0a,0x46,0x44,0x49,0x54,0x42,0x52,0x52,0x6b,0x6d,0x4d,0x73,0x51,0x4d,0x35,0x6d,
0x6d,0x70,0x4e,0x46,0x5a,0x64,0x69,0x50,0x37,0x54,0x4d,0x49,0x6f,0x4c,0x6c,0x65,
0x4a,0x73,0x5a,0x49,0x6a,0x56,0x72,0x46,0x78,0x67,0x4f,0x5a,0x69,0x34,0x79,0x41,
0x6f,0x45,0x34,0x78,0x51,0x77,0x66,0x66,0x2b,0x67,0x71,0x2f,0x78,0x4e,0x46,0x67,
0x45,0x0a,0x6a,0x65,0x46,0x6f,0x62,0x6b,0x4d,0x49,0x4e,0x39,0x4e,0x65,0x62,0x57,
0x55,0x4c,0x52,0x5a,0x67,0x48,0x59,0x44,0x53,0x38,0x70,0x63,0x74,0x33,0x42,0x62,
0x6d,0x37,0x58,0x6a,0x71,0x33,0x34,0x57,0x32,0x50,0x34,0x33,0x59,0x47,0x65,0x43,
0x67,0x5a,0x2f,0x30,0x44,0x45,0x59,0x7a,0x6a,0x6e,0x48,0x64,0x2f,0x59,0x56,0x41,
0x30,0x34,0x0a,0x76,0x49,0x73,0x42,0x73,0x64,0x39,0x57,0x4a,0x77,0x62,0x6f,0x76,
0x79,0x4b,0x31,0x55,0x66,0x69,0x4d,0x41,0x51,0x4b,0x42,0x67,0x51,0x43,0x49,0x36,
0x45,0x72,0x65,0x61,0x63,0x47,0x73,0x2f,0x41,0x43,0x75,0x59,0x36,0x34,0x73,0x45,
0x51,0x4c,0x35,0x55,0x6a,0x49,0x57,0x7a,0x35,0x61,0x43,0x39,0x54,0x79,0x2f,0x55,
0x68,0x4f,0x57,0x0a,0x39,0x32,0x62,0x68,0x56,0x4e,0x34,0x46,0x54,0x61,0x69,0x52,
0x41,0x65,0x6d,0x30,0x56,0x75,0x4b,0x47,0x57,0x36,0x48,0x4c,0x51,0x79,0x44,0x5a,
0x73,0x71,0x2f,0x4d,0x64,0x6c,0x36,0x4e,0x45,0x71,0x50,0x67,0x46,0x69,0x7a,0x75,
0x2f,0x36,0x68,0x68,0x6b,0x62,0x42,0x59,0x65,0x61,0x44,0x72,0x6c,0x51,0x41,0x46,
0x36,0x5a,0x2f,0x7a,0x0a,0x4f,0x73,0x39,0x54,0x48,0x33,0x61,0x57,0x4b,0x5a,0x78,
0x72,0x74,0x46,0x50,0x5a,0x6b,0x6f,0x46,0x6c,0x30,0x73,0x38,0x64,0x66,0x59,0x75,
0x67,0x36,0x45,0x44,0x67,0x76,0x73,0x50,0x32,0x62,0x41,0x55,0x65,0x61,0x58,0x4b,
0x73,0x38,0x62,0x43,0x52,0x42,0x6e,0x4b,0x43,0x53,0x49,0x75,0x38,0x45,0x4f,0x47,
0x55,0x33,0x31,0x59,0x4c,0x0a,0x57,0x63,0x73,0x6f,0x70,0x51,0x4b,0x42,0x67,0x44,
0x49,0x4c,0x2b,0x70,0x54,0x47,0x66,0x53,0x65,0x7a,0x66,0x69,0x70,0x78,0x4f,0x32,
0x6f,0x6d,0x56,0x55,0x41,0x47,0x4e,0x7a,0x32,0x68,0x4d,0x35,0x54,0x68,0x69,0x4e,
0x44,0x5a,0x52,0x68,0x33,0x67,0x74,0x37,0x6e,0x54,0x4c,0x45,0x4a,0x39,0x77,0x58,
0x65,0x4b,0x30,0x6c,0x46,0x47,0x0a,0x4b,0x72,0x75,0x67,0x43,0x6b,0x70,0x37,0x33,
0x43,0x53,0x37,0x6c,0x34,0x71,0x64,0x56,0x74,0x42,0x6b,0x78,0x75,0x4f,0x70,0x70,
0x75,0x52,0x75,0x73,0x37,0x48,0x30,0x39,0x68,0x72,0x46,0x35,0x73,0x4f,0x5a,0x61,
0x65,0x68,0x6c,0x79,0x32,0x7a,0x4b,0x53,0x66,0x35,0x7a,0x4d,0x46,0x4b,0x69,0x70,
0x6e,0x4e,0x71,0x71,0x42,0x71,0x7a,0x0a,0x44,0x68,0x79,0x78,0x79,0x79,0x74,0x46,
0x6e,0x73,0x49,0x61,0x71,0x2b,0x56,0x6f,0x4d,0x31,0x4e,0x35,0x32,0x50,0x5a,0x64,
0x6d,0x6e,0x62,0x4e,0x6f,0x31,0x6d,0x71,0x6b,0x43,0x34,0x65,0x48,0x32,0x6f,0x4d,
0x2b,0x56,0x56,0x5a,0x49,0x35,0x57,0x54,0x66,0x36,0x66,0x33,0x0a,0x2d,0x2d,0x2d,
0x2d,0x2d,0x45,0x4e,0x44,0x20,0x52,0x53,0x41,0x20,0x50,0x52,0x49,0x56,0x41,0x54,
0x45,0x20,0x4b,0x45,0x59,0x2d,0x2d,0x2d,0x2d,0x2d,0x0a};

unsigned short rsakey_pub[] = {
0x2d,0x2d,0x2d,0x2d,0x2d,0x42,0x45,0x47,0x49,0x4e,0x20,0x50,0x55,0x42,0x4c,0x49,
0x43,0x20,0x4b,0x45,0x59,0x2d,0x2d,0x2d,0x2d,0x2d,0x0a,0x4d,0x49,0x49,0x42,0x49,
0x6a,0x41,0x4e,0x42,0x67,0x6b,0x71,0x68,0x6b,0x69,0x47,0x39,0x77,0x30,0x42,0x41,
0x51,0x45,0x46,0x41,0x41,0x4f,0x43,0x41,0x51,0x38,0x41,0x4d,0x49,0x49,0x42,0x43,
0x67,0x4b,0x43,0x41,0x51,0x45,0x41,0x74,0x68,0x70,0x41,0x45,0x78,0x4b,0x57,0x4e,
0x49,0x4c,0x34,0x49,0x2f,0x49,0x35,0x6e,0x46,0x66,0x41,0x0a,0x57,0x55,0x73,0x43,
0x48,0x34,0x54,0x44,0x46,0x47,0x68,0x62,0x76,0x4e,0x65,0x4f,0x77,0x72,0x38,0x43,
0x73,0x57,0x51,0x4d,0x6a,0x69,0x31,0x42,0x5a,0x49,0x46,0x71,0x42,0x43,0x77,0x69,
0x75,0x41,0x51,0x7a,0x36,0x50,0x36,0x32,0x55,0x35,0x71,0x61,0x5a,0x5a,0x79,0x71,
0x4d,0x30,0x30,0x5a,0x45,0x33,0x4d,0x5a,0x38,0x63,0x31,0x6b,0x0a,0x74,0x66,0x76,
0x4c,0x78,0x4a,0x68,0x37,0x62,0x34,0x69,0x62,0x37,0x36,0x52,0x6c,0x48,0x66,0x72,
0x78,0x6d,0x54,0x44,0x4b,0x66,0x79,0x6b,0x37,0x79,0x30,0x75,0x4c,0x34,0x2f,0x55,
0x57,0x53,0x44,0x52,0x31,0x50,0x4d,0x65,0x59,0x4c,0x4c,0x65,0x6e,0x42,0x47,0x4f,
0x64,0x53,0x4a,0x59,0x78,0x6d,0x7a,0x44,0x79,0x44,0x70,0x76,0x34,0x0a,0x6a,0x41,
0x44,0x33,0x30,0x36,0x41,0x58,0x6d,0x30,0x64,0x73,0x66,0x74,0x74,0x71,0x55,0x45,
0x6a,0x58,0x6f,0x4b,0x6d,0x68,0x36,0x4b,0x53,0x48,0x69,0x70,0x56,0x31,0x4d,0x45,
0x66,0x55,0x66,0x2b,0x56,0x34,0x6c,0x47,0x62,0x58,0x79,0x4e,0x39,0x72,0x53,0x6e,
0x4e,0x36,0x4e,0x37,0x6f,0x4f,0x7a,0x37,0x6c,0x6f,0x48,0x2b,0x47,0x55,0x0a,0x4f,
0x6d,0x4d,0x55,0x49,0x55,0x54,0x5a,0x70,0x63,0x56,0x38,0x51,0x77,0x2b,0x30,0x4e,
0x46,0x44,0x69,0x75,0x66,0x66,0x66,0x75,0x79,0x71,0x6e,0x64,0x70,0x74,0x49,0x4c,
0x66,0x6c,0x59,0x70,0x41,0x4f,0x47,0x74,0x76,0x6b,0x49,0x44,0x42,0x54,0x37,0x51,
0x6d,0x63,0x66,0x77,0x2b,0x4d,0x69,0x30,0x57,0x70,0x76,0x68,0x43,0x63,0x77,0x0a,
0x6b,0x2b,0x34,0x4a,0x79,0x64,0x75,0x32,0x39,0x51,0x30,0x6d,0x41,0x57,0x36,0x45,
0x6f,0x30,0x51,0x31,0x75,0x79,0x30,0x4c,0x71,0x57,0x2f,0x64,0x6c,0x61,0x74,0x69,
0x44,0x57,0x66,0x2f,0x38,0x53,0x51,0x61,0x49,0x77,0x45,0x42,0x54,0x46,0x43,0x63,
0x51,0x69,0x52,0x56,0x32,0x67,0x79,0x42,0x77,0x52,0x57,0x67,0x67,0x31,0x44,0x77,
0x0a,0x6f,0x77,0x49,0x44,0x41,0x51,0x41,0x42,0x0a,0x2d,0x2d,0x2d,0x2d,0x2d,0x45,
0x4e,0x44,0x20,0x50,0x55,0x42,0x4c,0x49,0x43,0x20,0x4b,0x45,0x59,0x2d,0x2d,0x2d,
0x2d,0x2d,0x0a};

unsigned char gdbuff[8092];
int gdpos = 0;

bool datacb(unsigned char*d, size_t len)
{
  fprintf(stderr, "Got : %d\n", len);

  if ((gdpos+len)>sizeof(gdbuff))
  {
    fprintf(stderr, "Got X: %d\n", len);
    return false;
  }
  memcpy(gdbuff+gdpos, d, len);
  gdpos+=len;
  return true;
}
  
bool datacb2(unsigned char*d, size_t len)
{
  printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n[");
  for(int i=0;i<len;++i)
  {
    printf("%c", d[i]);
  }
  printf("]\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

  return true;
}

int main()
{
  char *text = "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
  //char *text = "1234567890";

#if 0
/******************* pub key *******************/
{
  RSAPubEncoder pubenc((unsigned char*)gkbuff, st.st_size);
  pubenc.setDataCallback(datacb);

  pubenc.execute((unsigned char*)text, strlen(text));
}

fprintf(stderr, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX gdpos=%d\n", gdpos);

/******************* pri key *******************/
{
  RSAPriDecoder pridec(gkbuff, st.st_size);
  pridec.setDataCallback(datacb2);
  pridec.execute(gdbuff, gdpos);

  fclose(filep);
}

fprintf(stderr, "\nNext test:\n");

#endif

gdpos=0;

/******************* pri key *******************/
{
  unsigned char buff[8192];
  size_t len = sizeof(rsakey);
  for(int i=0; i<len; ++i)
  {
    buff[i] = (char)rsakey[i];
  }
  RSAPriEncoder prienc(buff, len);
  prienc.setDataCallback(datacb);
  prienc.execute((unsigned char*)text, strlen(text));

}

fprintf(stderr, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX gdpos=%d\n", gdpos);

/******************* pub key *******************/
{
  unsigned char buff[8192];
  size_t len = sizeof(rsakey_pub);
  for(int i=0; i<len; ++i)
  {
    buff[i] = (char)rsakey_pub[i];
  }

  RSAPubDecoder pubdec(buff, len);
  pubdec.setDataCallback(datacb2);

  pubdec.execute(gdbuff, gdpos);

}

  return 0;
}

