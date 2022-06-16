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
        name: "bar" |> A.of_public |> U.as_untyped,
        default: None,
        type_: None,
      }
      |> U.as_int,
      A.{
        name: "fizz" |> A.of_public |> U.as_untyped,
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
      ("zip" |> A.of_public |> U.as_untyped, 3 |> U.int_prim)
      |> A.of_var
      |> U.as_nil,
      ("zap" |> A.of_public |> U.as_untyped, 4 |> U.int_prim)
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

let __inline_view = (
  "foo" |> A.of_public,
  (
    [
      A.{
        name: "bar" |> A.of_public |> U.as_untyped,
        default: None,
        type_: None,
      }
      |> U.as_int,
      A.{
        name: "fizz" |> A.of_public |> U.as_untyped,
        default: Some(3 |> U.int_prim),
        type_: None,
      }
      |> U.as_int,
    ],
    [
      (
        "bar" |> A.of_public |> A.of_id |> U.as_int,
        "fizz" |> A.of_public |> A.of_id |> U.as_int,
      )
      |> A.of_add_op
      |> U.as_int
      |> A.of_inline_expr
      |> U.as_untyped,
    ]
    |> A.of_frag
    |> A.of_jsx
    |> U.as_element,
  )
  |> A.of_view,
);

let __multiline_view = (
  "buzz" |> A.of_public,
  (
    [],
    [
      ("zip" |> A.of_public |> U.as_untyped, 3 |> U.int_prim)
      |> A.of_var
      |> U.as_nil,
      ("zap" |> A.of_public |> U.as_untyped, 4 |> U.int_prim)
      |> A.of_var
      |> U.as_nil,
      [
        (
          "zip" |> A.of_public |> A.of_id |> U.as_int,
          "zap" |> A.of_public |> A.of_id |> U.as_int,
        )
        |> A.of_mult_op
        |> U.as_int
        |> A.of_inline_expr
        |> U.as_untyped,
      ]
      |> A.of_frag
      |> A.of_jsx
      |> U.as_element
      |> A.of_expr
      |> U.as_element,
    ]
    |> A.of_closure
    |> U.as_int,
  )
  |> A.of_view,
);

let __style = (
  "foo" |> A.of_public,
  (
    [
      A.{
        name: "bar" |> A.of_public |> U.as_untyped,
        default: None,
        type_: None,
      }
      |> U.as_int,
      A.{
        name: "fizz" |> A.of_public |> U.as_untyped,
        default: Some(3 |> U.int_prim),
        type_: None,
      }
      |> U.as_int,
    ],
    [
      (
        A.ID("hero" |> A.of_public |> U.as_untyped),
        [
          ("width" |> A.of_public |> U.as_untyped, U.int_prim(10))
          |> U.as_untyped,
        ],
      )
      |> U.as_untyped,
      (
        A.Class("button" |> A.of_public |> U.as_untyped),
        [
          ("height" |> A.of_public |> U.as_untyped, U.int_prim(2))
          |> U.as_untyped,
        ],
      )
      |> U.as_untyped,
    ],
  )
  |> A.of_style,
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
    "pp_declaration() - empty enumerated"
    >: (
      () =>
        _assert_declaration(
          "enum Account = | ;",
          (A.of_public("Account"), A.of_enum([])),
        )
    ),
    "pp_declaration() - multiline enumerated"
    >: (
      () =>
        _assert_declaration(
          "enum Digits =
  | Zero
  | One
  | Two
  | Three
  | Four
  | Five
  | Six
  | Seven
  | Eight
  | Nine;",
          (
            A.of_public("Digits"),
            A.of_enum([
              ("Zero" |> A.of_public |> U.as_untyped, []),
              ("One" |> A.of_public |> U.as_untyped, []),
              ("Two" |> A.of_public |> U.as_untyped, []),
              ("Three" |> A.of_public |> U.as_untyped, []),
              ("Four" |> A.of_public |> U.as_untyped, []),
              ("Five" |> A.of_public |> U.as_untyped, []),
              ("Six" |> A.of_public |> U.as_untyped, []),
              ("Seven" |> A.of_public |> U.as_untyped, []),
              ("Eight" |> A.of_public |> U.as_untyped, []),
              ("Nine" |> A.of_public |> U.as_untyped, []),
            ]),
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
    "pp_declaration() - inline view"
    >: (
      () =>
        _assert_declaration(
          "view foo(bar, fizz = 3) -> <>
  {bar + fizz}
</>;",
          __inline_view,
        )
    ),
    "pp_declaration() - multiline view"
    >: (
      () =>
        _assert_declaration(
          "view buzz -> {
  let zip = 3;
  let zap = 4;
  <>
    {zip * zap}
  </>;
}",
          __multiline_view,
        )
    ),
    "pp_declaration() - style"
    >: (
      () =>
        _assert_declaration(
          "style foo(bar, fizz = 3) -> {
  #hero {
    width: 10;
  }
  .button {
    height: 2;
  }
}",
          __style,
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
