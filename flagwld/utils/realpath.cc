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


// Copyright (C) 2006, by Timmy <timmy1992@gmail.com>
//
// This file may be distributed under the terms of the Q Public License
// version 1.0 as appearing in the file LICENSE.QPL included in the
// packaging of this file.

#include <string>
#include <assert.h>

// Return canonical path name of path.
//
// realpath resolves references to '/./', '/../' and extra '/' characters in
// the string named by path and returns the canonicalized absolute pathname.
// The resulting path will have no '/./' or '/../' components, also no trailing ones.
// Nor will it  end on a slash: if the result is the root then the returned path is empty,
// and unless the result is empty, it will always start with a slash.

#include <flagwld/utils/realpath.h>

flagwld::string flagwld::utils::realpath(flagwld::string const &path, flagwld::string const &curdir)
{
	flagwld::string full_path, result;
	if (path.length() > 0)
		full_path = (path[0] == '/' || curdir.length() == 0)
			? path : curdir + "/" + path;
	else
		full_path = curdir;
	assert(full_path[0] == '/');

	flagwld::string::iterator slash1 = full_path.begin();
	for (flagwld::string::iterator slash2 = slash1; slash1 != full_path.end(); slash1 = slash2) {
		while (slash2 != full_path.end() && *++slash2 != '/'); // Find the next slash.
		// Now we have:
		// /usr/src/cppgraph/cppgraph-objdir/src/lll
		//         ^        ^
		//      slash1    slash2
		// or
		// /usr/src/cppgraph/cppgraph-objdir/src/lll
		//                                      ^   ^
		//                                  slash1  slash2
		// result is either empty, or starts with a slash.
		size_t dirlen = slash2 - slash1;
		// Ignore trailing slashes, sequences of '//', sequences of '/./' and a trailing '/.'.
		if (dirlen == 1 || (dirlen == 2 && slash1[1] == '.'))
			continue;
		// Process sequences of '/../' or a trailing '/..'.
		if (dirlen == 3 && slash1[1] == '.' && slash1[2] == '.') {
			flagwld::string::iterator iter = result.end();
			while (iter != result.begin() && *--iter != '/');
			result.erase(iter, result.end());
			continue;
		}
		result += full_path.substr(slash1 - full_path.begin(), dirlen);
	}
	return result.empty() ? "/" : result;
}
