#include "lisp.hpp"

using namespace lisp;

int main() {
    using cons1 = Cons<IntAtom<1>, Cons<IntAtom<2>, CharAtom<'h'>>>;
    static_assert(Value<Head<cons1>> == 1);
    static_assert(Value<Head<Tail<cons1>>> == 2);
    Println<cons1>();

    using list1 = List<IntAtom<1>, CharAtom<'c'>, BoolAtom<false>>;
    using str1 = String<'h', 'e', 'l', 'l', 'o'>;
    Println<list1>();
    Println<str1>();
    static_assert(Equal<Nul, LastElem<str1>>);

    return 0;
}
