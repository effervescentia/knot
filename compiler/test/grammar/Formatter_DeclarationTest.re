open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Formatter = Grammar.Formatter;

let __int_const = ("ABC" |> of_public, 123 |> int_prim |> of_const);
let __bool_const = ("DEF" |> of_public, true |> bool_prim |> of_const);

let __inline_function = (
  "foo" |> of_public,
  (
    [
      {name: "bar" |> of_public |> as_raw_node, default: None, type_: None}
      |> as_int,
      {
        name: "fizz" |> of_public |> as_raw_node,
        default: Some(3 |> int_prim),
        type_: None,
      }
      |> as_int,
    ],
    (
      "bar" |> of_public |> as_int |> of_id |> as_int,
      "fizz" |> of_public |> as_int |> of_id |> as_int,
    )
    |> of_add_op
    |> as_int,
  )
  |> of_func,
);

let __multiline_function = (
  "buzz" |> of_public,
  (
    [],
    [
      ("zip" |> of_public |> as_raw_node, 3 |> int_prim) |> of_var |> as_nil,
      ("zap" |> of_public |> as_raw_node, 4 |> int_prim) |> of_var |> as_nil,
      (
        "zip" |> of_public |> as_int |> of_id |> as_int,
        "zap" |> of_public |> as_int |> of_id |> as_int,
      )
      |> of_mult_op
      |> as_int
      |> of_expr
      |> as_int,
    ]
    |> of_closure
    |> as_int,
  )
  |> of_func,
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
          (
            "foo" |> of_public,
            nil |> as_nil |> of_prim |> as_nil |> of_const,
          ),
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
