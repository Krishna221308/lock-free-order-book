#include "lob/intrusive_list.hpp"
#include <cassert>
#include <iostream>

struct IntNode : lob::IntrusiveListNode<IntNode> {
    int value;
    explicit IntNode(int v) : value(v) {}
};

int main() {
    lob::IntrusiveList<IntNode> list;

    IntNode n1(10);
    IntNode n2(20);
    IntNode n3(30);
    IntNode n4(40);
    IntNode n5(50);

    list.push_back(&n1);
    list.push_back(&n2);
    list.push_back(&n3);
    list.push_back(&n4);
    list.push_back(&n5);

    assert(list.size() == 5);
    
    list.remove(&n3);
    assert(list.size() == 4);

    int expected_fwd[] = {10, 20, 40, 50};
    int i = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
        assert((*it)->value == expected_fwd[i]);
        ++i;
    }

    // 4. Confirm remaining nodes are correctly linked backwards
    int expected_bwd[] = {50, 40, 20, 10};
    int j = 0;
    for (auto it = list.rbegin(); it != list.rend(); ++it) {
        assert((*it)->value == expected_bwd[j]);
        j++;
    }
    assert(j == 4);
    std::cout << "IntrusiveList tests passed successfully!\n";
    return 0;
}