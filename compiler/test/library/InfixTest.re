open Kore;

let _assert_opt_int_list =
  Alcotest.(check(option(list(int)), "option int list matches"));

let suite =
  "Library.Infix"
  >::: [
    "(%)"
    >: (
      () =>
        [
          (
            "{[(a)]}",
            (Fmt.str("(%s)") % Fmt.str("[%s]") % Fmt.str("{%s}"))("a"),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "(@?)"
    >: (
      () =>
        [
          (Some([]), Some([]) @? Some([])),
          (Some([1, 2, 3, 4]), Some([1, 2]) @? Some([3, 4])),
          (Some([1, 2]), Some([1, 2]) @? None),
          (Some([3, 4]), None @? Some([3, 4])),
          (None, None @? None),
        ]
        |> Assert.test_many(_assert_opt_int_list)
    ),
    "(|?:)"
    >: (
      () =>
        [(1, Some(1) |?: 2), (2, None |?: 2)] |> Assert.(test_many(int))
    ),
    "(|!:)"
    >: (
      () =>
        [(1, Some(1) |!: (() => 2)), (2, None |!: (() => 2))]
        |> Assert.(test_many(int))
    ),
    "(|?<)"
    >: (
      () =>
        [
          (Some(3), Some(1) |?< (x => Some(x + 2))),
          (None, None |?< (x => Some(x + 2))),
          (None, Some(1) |?< (_ => None)),
        ]
        |> Assert.(test_many(opt_int))
    ),
    "(|?>)"
    >: (
      () =>
        [(Some(3), Some(1) |?> (+)(2)), (None, None |?> (+)(2))]
        |> Assert.(test_many(opt_int))
    ),
    "(~@)"
    >: (
      () => [("foo", "foo" |> ~@Fmt.string)] |> Assert.(test_many(string))
    ),
  ];
