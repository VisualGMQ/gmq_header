#include "sparse_sets.hpp"

int main() {
    SparseSets<uint32_t, 10> set;

    set.Add(1);
    set.Add(6);
    set.Add(9);
    set.Add(2);
    set.Add(13);

    assert(set.Contain(1));
    assert(set.Contain(6));
    assert(set.Contain(9));
    assert(set.Contain(2));
    assert(set.Contain(13));
    assert(!set.Contain(0));
    assert(!set.Contain(15));
    assert(!set.Contain(10));
    assert(!set.Contain(7));

    set.Remove(2);
    set.Remove(0);
    set.Remove(9);
    assert(!set.Contain(2));
    assert(!set.Contain(9));

    set.Remove(13);
    assert(!set.Contain(13));
}