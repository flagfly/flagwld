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
   PrefixTree from https://github.com/spakai/prefix_tree.git
*/

#ifndef FLAGWLD_UTILS_PREFIXTREE_H
#define FLAGWLD_UTILS_PREFIXTREE_H

#include <flagwld/base/Types.h>

#include <boost/unordered_map.hpp>
#include <boost/any.hpp>

#include <vector>

#include <sys/types.h>

namespace flagwld
{
namespace utils
{

struct Node {
    char chr;
    std::vector<Node*> child;
    int numberOfNodes;

    boost::any context;

    Node(int _numberOfNodes);
    ~Node();
};

class PrefixTree {
    public:
        PrefixTree();
        ~PrefixTree();
        void loadMapping();
        Node* getRoot();
        int determineIndex(char value);

        bool insert(flagwld::string & word, const boost::any& context);
        //const boost::any& search(const flagwld::string & word) __attribute__((deprecated)); //damaged, maybe core when concurrent request
        const boost::any& search(const flagwld::string & word);
        boost::any* searchMutable(const flagwld::string & word);

    private:
        Node *root;   
        boost::unordered_map<char, int> charToIndexMap;
        bool nodeDoesNotExist(Node* currentNode);
        Node* allocateMemoryAndAssignValue(Node * currentNode, char value); 
};

}
}

#endif //FLAGWLD_UTILS_PREFIXTREE_H
