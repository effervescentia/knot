open Kore;

let suite =
  "Library.List"
  >::: [
    "is_empty()"
    >: (
      () => {
        List.is_empty([]) |> Assert.true_;
        List.is_empty([1, 2, 3]) |> Assert.false_;
      }
    ),
    "incl() - empty" >: (() => Assert.int_list([0], List.incl(0, []))),
    "incl() - item exists"
    >: (() => Assert.int_list([1, 2, 3], List.incl(2, [1, 2, 3]))),
    "incl() - item does not exist"
    >: (() => Assert.int_list([0, 1, 2, 3], List.incl(0, [1, 2, 3]))),
    "excl() - empty" >: (() => Assert.int_list([], List.excl(0, []))),
    "excl() - item exists"
    >: (() => Assert.int_list([1, 3], List.excl(2, [1, 2, 3]))),
    "excl() - item does not exist"
    >: (() => Assert.int_list([1, 2, 3], List.excl(4, [1, 2, 3]))),
    "excl_all() - empty"
    >: (() => Assert.int_list([], List.excl_all([1, 2], []))),
    "excl_all() - exclude some"
    >: (() => Assert.int_list([1], List.excl_all([2, 3], [1, 2, 3]))),
    "excl_all() - exclude none"
    >: (
      () => Assert.int_list([1, 2, 3], List.excl_all([4, 5], [1, 2, 3]))
    ),
    "excl_all() - empty excludes"
    >: (() => Assert.int_list([1, 2, 3], List.excl_all([], [1, 2, 3]))),
    "uniq_by()"
    >: (
      () =>
        Assert.string_list(
          ["blueberry", "apple"],
          List.uniq_by(
            (l, r) => String.sub(l, 0, 1) == String.sub(r, 0, 1),
            ["apple", "apricot", "blueberry"],
          ),
        )
    ),
    "compare_members() - empty lists"
    >: (() => Assert.true_(List.compare_members([], []))),
    "compare_members() - no difference"
    >: (() => Assert.true_(List.compare_members([1], [1]))),
    "compare_members() - different order"
    >: (() => Assert.true_(List.compare_members([3, 1, 2], [2, 1, 3]))),
    "compare_members() - different number of occurrences"
    >: (() => Assert.true_(List.compare_members([2, 2, 1, 2], [1, 1, 2]))),
    "compare_members() - different members"
    >: (() => Assert.false_(List.compare_members([1], [2]))),
    "compare_members() - each has members the other does not"
    >: (() => Assert.false_(List.compare_members([1, 2], [2, 3]))),
    "compare_members() - some overlap of members"
    >: (() => Assert.false_(List.compare_members([2], [2, 3]))),
    "ends() - one item" >: (() => Assert.int_pair((1, 1), List.ends([1]))),
    "ends() - multiple items"
    >: (() => Assert.int_pair((1, 4), List.ends([1, 2, 3, 4]))),
    "ends() - raise NoListMembers"
    >: (
      () =>
        switch (List.ends([])) {
        | exception List.NoListMembers => ()
        | _ => Alcotest.fail("should raise NoListMembers")
        }
    ),
    "repeat()" >: (() => Assert.int_list([1, 1, 1], List.repeat(3, 1))),
    "last() - empty" >: (() => Assert.opt_int(None, List.last([]))),
    "last() - not empty"
    >: (() => Assert.opt_int(Some(3), List.last([1, 2, 3]))),
    "divide() - empty"
    >: (() => Assert.pair_int_list(([], []), List.divide([]))),
    "divide() - even list length"
    >: (
      () =>
        Assert.pair_int_list(([1, 2], [3, 4]), List.divide([1, 2, 3, 4]))
    ),
    "divide() - uneven list length"
    >: (() => Assert.pair_int_list(([1], [2, 3]), List.divide([1, 2, 3]))),
    "divide() - one item"
    >: (() => Assert.pair_int_list(([1], []), List.divide([1]))),
  ];
