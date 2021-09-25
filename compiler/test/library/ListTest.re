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
    "incl()"
    >: (
      () =>
        [
          ([0], List.incl(0, [])),
          ([1, 2, 3], List.incl(2, [1, 2, 3])),
          ([0, 1, 2, 3], List.incl(0, [1, 2, 3])),
        ]
        |> Assert.(test_many(int_list))
    ),
    "excl()"
    >: (
      () =>
        [([], List.excl(0, [])), ([1, 3], List.excl(2, [1, 2, 3]))]
        |> Assert.(test_many(int_list))
    ),
    "excl_all()"
    >: (
      () =>
        [
          ([], List.excl_all([1, 2], [])),
          ([1], List.excl_all([2, 3], [1, 2, 3])),
          ([1, 2, 3], List.excl_all([], [1, 2, 3])),
        ]
        |> Assert.(test_many(int_list))
    ),
    "uniq_by()"
    >: (
      () =>
        [
          (
            ["blueberry", "apple"],
            List.uniq_by(
              (l, r) => String.sub(l, 0, 1) == String.sub(r, 0, 1),
              ["apple", "apricot", "blueberry"],
            ),
          ),
        ]
        |> Assert.(test_many(string_list))
    ),
    "compare_members()"
    >: (
      () =>
        [
          (true, List.compare_members([], [])),
          (true, List.compare_members([1], [1])),
          (true, List.compare_members([1, 2], [2, 1])),
          (true, List.compare_members([3, 1, 2], [2, 1, 3])),
          (true, List.compare_members([3, 1, 2], [2, 1, 3])),
          (true, List.compare_members([2, 2, 1, 2], [1, 1, 2])),
          (false, List.compare_members([1], [2])),
          (false, List.compare_members([1, 2], [2, 3])),
          (false, List.compare_members([2], [2, 3])),
        ]
        |> Assert.(test_many(bool))
    ),
    "ends()"
    >: (
      () =>
        [((1, 1), List.ends([1])), ((1, 4), List.ends([1, 2, 3, 4]))]
        |> Assert.(test_many(int_pair))
    ),
    "ends() - raise NoListMembers"
    >: (
      () =>
        switch (List.ends([])) {
        | exception List.NoListMembers => ()
        | _ => Alcotest.fail("should raise NoListMembers")
        }
    ),
    "repeat()"
    >: (
      () =>
        [([1, 1, 1], List.repeat(3, 1))] |> Assert.(test_many(int_list))
    ),
    "last()"
    >: (
      () =>
        [(Some(3), List.last([1, 2, 3])), (None, List.last([]))]
        |> Assert.(test_many(opt_int))
    ),
    "divide()"
    >: (
      () =>
        [
          (([1, 2], [3, 4]), List.divide([1, 2, 3, 4])),
          (([1], [2, 3]), List.divide([1, 2, 3])),
          (([1], []), List.divide([1])),
          (([], []), List.divide([])),
        ]
        |> Assert.(test_many(pair_int_list))
    ),
  ];
