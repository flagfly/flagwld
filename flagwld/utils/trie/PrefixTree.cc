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

#include <flagwld/utils/trie/PrefixTree.h>

#include <memory>
#include <exception>

using namespace flagwld;
using namespace flagwld::utils;

Node::Node(int _numberOfNodes=36) : numberOfNodes(_numberOfNodes) { /*0-9A-Z*/
  for(int i{0}; i < numberOfNodes; i++) {
    child.push_back(nullptr);
  }
}

Node::~Node() {
  for(int i{0}; i < numberOfNodes; i++) {
    if(child[i] != nullptr) {
      delete child[i];
    }
  }
}

PrefixTree::PrefixTree() {
    root = new Node();
    loadMapping();
}

void PrefixTree::loadMapping() {
    flagwld::string chars="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i{0};
    for(auto & chr : chars) {
        charToIndexMap.emplace(chr, i++);        
    } 
}

PrefixTree::~PrefixTree() {
    delete root;
}

Node* PrefixTree::getRoot() {
    return root;
}

int PrefixTree::determineIndex(char value) {
    auto it = charToIndexMap.find(value);
    return it == charToIndexMap.end() ? -1 : it->second;
}

bool PrefixTree::nodeDoesNotExist(Node* node) {
    if(node == nullptr) {
        return true;
    } 

    return false;
} 

Node* PrefixTree::allocateMemoryAndAssignValue(Node * node, char value) {
    node = new Node();
    node->chr = value;
    return node;
}

bool PrefixTree::insert(flagwld::string & word, const boost::any& context) {
    int index{0};
    Node* currentNode = root;
    for(auto & chr : word) {
        index = determineIndex(chr);
        if (index < 0) {
            return false;
        }
        if(nodeDoesNotExist(currentNode->child[index])) {
            currentNode->child[index] = allocateMemoryAndAssignValue(currentNode->child[index], chr);
            currentNode = currentNode->child[index];
        } else {
            currentNode = currentNode->child[index];
        }
    }

    if (!currentNode->context.empty()) {
        return false;
    }

    currentNode->context = context;

    return true;
}

const boost::any& PrefixTree::search(const flagwld::string & word) {
    int index{0};
    Node* currentNode = root;
    boost::any* contextPtr = &(root->context);
    for(auto & chr : word) {
        index = determineIndex(chr);
        if (index < 0) {
            break;
        }
        if(nodeDoesNotExist(currentNode->child[index])) {
            break;
        } else {
            currentNode = currentNode->child[index];
            if (!currentNode->context.empty())
            {
                contextPtr = &(currentNode->context);
            }
        }
    }

    if(!contextPtr->empty()) {
        return *contextPtr;
    } else {
        throw std::out_of_range("index out of range");
    }
}

boost::any* PrefixTree::searchMutable(const flagwld::string & word) {
    int index{0};
    Node* currentNode = root;
    boost::any* contextPtr = &(root->context);
    for(auto & chr : word) {
        index = determineIndex(chr);
        if (index < 0) {
            break;
        }
        if(nodeDoesNotExist(currentNode->child[index])) {
            break;
        } else {
            currentNode = currentNode->child[index];
            if (!currentNode->context.empty())
            {
                contextPtr = &(currentNode->context);
            }
        }
    }

    if(!contextPtr->empty()) {
        return contextPtr;
    } else {
        throw std::out_of_range("index out of range");
    }
}
