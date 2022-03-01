open Kore;

module Formatter = Grammar.Formatter;
module U = Util.ResultUtil;

let __int_const = ("ABC" |> A.of_public, 123 |> U.int_prim |> A.of_const);
let __bool_const = ("DEF" |> A.of_public, true |> U.bool_prim |> A.of_const);

let __inline_function = (
  "foo" |> A.of_public,
  (
    [
      A.{
        name: "bar" |> A.of_public |> U.as_raw_node,
        default: None,
        type_: None,
      }
      |> U.as_int,
      A.{
        name: "fizz" |> A.of_public |> U.as_raw_node,
        default: Some(3 |> U.int_prim),
        type_: None,
      }
      |> U.as_int,
    ],
    (
      "bar" |> A.of_public |> A.of_id |> U.as_int,
      "fizz" |> A.of_public |> A.of_id |> U.as_int,
    )
    |> A.of_add_op
    |> U.as_int,
  )
  |> A.of_func,
);

let __multiline_function = (
  "buzz" |> A.of_public,
  (
    [],
    [
      ("zip" |> A.of_public |> U.as_raw_node, 3 |> U.int_prim)
      |> A.of_var
      |> U.as_nil,
      ("zap" |> A.of_public |> U.as_raw_node, 4 |> U.int_prim)
      |> A.of_var
      |> U.as_nil,
      (
        "zip" |> A.of_public |> A.of_id |> U.as_int,
        "zap" |> A.of_public |> A.of_id |> U.as_int,
      )
      |> A.of_mult_op
      |> U.as_int
      |> A.of_expr
      |> U.as_int,
    ]
    |> A.of_closure
    |> U.as_int,
  )
  |> A.of_func,
);

let _assert_declaration = (expected, actual) =>
  Assert.string(expected, actual |> ~@Fmt.root(pp_declaration));
let _assert_declaration_list = (expected, actual) =>
  Assert.string(expected, actual |> ~@Fmt.root(pp_declaration_list));

let suite =
  "Grammar.Formatter | Declaration"
  >::: [
    "pp_declaration() - constant"
    >: (
      () =>
        _assert_declaration(
          "const foo = nil;",
          (A.of_public("foo"), A.of_const(U.nil_prim)),
        )
    ),
    "pp_declaration() - inline function"
    >: (
      () =>
        _assert_declaration(
          "func foo(bar, fizz = 3) -> bar + fizz;",
          __inline_function,
        )
    ),
    "pp_declaration() - multiline function"
    >: (
      () =>
        _assert_declaration(
          "func buzz -> {
  let zip = 3;
  let zap = 4;
  zip * zap;
}",
          __multiline_function,
        )
    ),
    "pp_declaration_list() - empty"
    >: (() => _assert_declaration_list("", [])),
    "pp_declaration_list() - single constant declaration"
    >: (() => _assert_declaration_list("const ABC = 123;", [__int_const])),
    "pp_declaration_list() - multiple constant declarations"
    >: (
      () =>
        _assert_declaration_list(
          "const DEF = true;
const ABC = 123;",
          [__bool_const, __int_const],
        )
    ),
    "pp_declaration_list() - constant and function declarations"
    >: (
      () =>
        _assert_declaration_list(
          "const DEF = true;

func foo(bar, fizz = 3) -> bar + fizz;

const ABC = 123;",
          [__bool_const, __inline_function, __int_const],
        )
    ),
    "pp_declaration_list() - cluster constants before function declaration"
    >: (
      () =>
        _assert_declaration_list(
          "const DEF = true;
const ABC = 123;

func foo(bar, fizz = 3) -> bar + fizz;",
          [__bool_const, __int_const, __inline_function],
        )
    ),
    "pp_declaration_list() - cluster constants after function declaration"
    >: (
      () =>
        _assert_declaration_list(
          "func foo(bar, fizz = 3) -> bar + fizz;

const DEF = true;
const ABC = 123;",
          [__inline_function, __bool_const, __int_const],
        )
    ),
  ];
