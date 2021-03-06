open Kore;

let suite =
  "Library - List"
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
      () => {
        List.compare_members([], []) |> Assert.true_;
        List.compare_members([1], [1]) |> Assert.true_;
        List.compare_members([1, 2], [2, 1]) |> Assert.true_;
        List.compare_members([3, 1, 2], [2, 1, 3]) |> Assert.true_;
        List.compare_members([3, 1, 2], [2, 1, 3]) |> Assert.true_;
        List.compare_members([2, 2, 1, 2], [1, 1, 2]) |> Assert.true_;
        List.compare_members([1], [2]) |> Assert.false_;
        List.compare_members([1, 2], [2, 3]) |> Assert.false_;
      }
    ),
    "ends()"
    >: (
      () =>
        [((1, 1), List.ends([1])), ((1, 4), List.ends([1, 2, 3, 4]))]
        |> Assert.(test_many(int_pair))
    ),
    "last()"
    >: (
      () =>
        [(Some(3), List.last([1, 2, 3])), (None, List.last([]))]
        |> Assert.(test_many(opt_int))
    ),
  ];
