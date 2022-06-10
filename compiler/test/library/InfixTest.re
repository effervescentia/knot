open Kore;

let _assert_opt_int_list =
  Alcotest.(check(option(list(int)), "option int list matches"));

let suite =
  "Library.Infix"
  >::: [
    "(%)"
    >: (
      () =>
        Assert.string(
          "{[(a)]}",
          (Fmt.str("(%s)") % Fmt.str("[%s]") % Fmt.str("{%s}"))("a"),
        )
    ),
    "(@?) - concat empty"
    >: (() => _assert_opt_int_list(Some([]), Some([]) @? Some([]))),
    "(@?) - concat"
    >: (
      () =>
        _assert_opt_int_list(
          Some([1, 2, 3, 4]),
          Some([1, 2]) @? Some([3, 4]),
        )
    ),
    "(@?) - select lhs"
    >: (() => _assert_opt_int_list(Some([1, 2]), Some([1, 2]) @? None)),
    "(@?) - select rhs"
    >: (() => _assert_opt_int_list(Some([3, 4]), None @? Some([3, 4]))),
    "(@?) - return empty" >: (() => _assert_opt_int_list(None, None @? None)),
    "(|?:) - has value" >: (() => Assert.int(1, Some(1) |?: 2)),
    "(|?:) - has no value" >: (() => Assert.int(2, None |?: 2)),
    "(|!:) - has value" >: (() => Assert.int(1, Some(1) |!: (() => 2))),
    "(|!:) - has no value" >: (() => Assert.int(2, None |!: (() => 2))),
    "(|?<) - has value"
    >: (() => Assert.opt_int(Some(3), Some(1) |?< (x => Some(x + 2)))),
    "(|?<) - has no value"
    >: (() => Assert.opt_int(None, None |?< (x => Some(x + 2)))),
    "(|?<) - transformer to none"
    >: (() => Assert.opt_int(None, Some(1) |?< (_ => None))),
    "(|?>) - has value"
    >: (() => Assert.opt_int(Some(3), Some(1) |?> (+)(2))),
    "(|?>) - has no value" >: (() => Assert.opt_int(None, None |?> (+)(2))),
    "(~@)" >: (() => Assert.string("foo", "foo" |> ~@Fmt.string)),
  ];
