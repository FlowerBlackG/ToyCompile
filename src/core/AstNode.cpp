// SPDX-License-Identifier: MulanPSL-2.0

/*

    Abstract Syntax Tree Node
    part of the ToyCompile project.

    created on 2022.11.2 at Tongji University.

*/

#include <core/AstNode.h>

using namespace std;
using namespace tc;

AstNode::~AstNode() {
    this->freeChildren();
}

void AstNode::freeChildren() {
    for (auto& child : children) {
        delete child;
    }

    children.clear();
}
