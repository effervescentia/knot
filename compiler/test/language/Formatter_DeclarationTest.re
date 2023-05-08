open Kore;

module Formatter = Language.Formatter;
module U = Util.ResultUtil;

let __int_const = ("ABC", 123 |> U.int_prim |> Declaration.of_constant);
let __bool_const = ("DEF", true |> U.bool_prim |> Declaration.of_constant);

let __inline_function = (
  "foo",
  (
    [
      (U.as_untyped("bar"), None, None) |> U.as_int,
      (U.as_untyped("fizz"), None, Some(3 |> U.int_prim)) |> U.as_int,
    ],
    (
      "bar" |> Expression.of_identifier |> U.as_int,
      "fizz" |> Expression.of_identifier |> U.as_int,
    )
    |> Expression.of_add_op
    |> U.as_int,
  )
  |> Declaration.of_function,
);

let __multiline_function = (
  "buzz",
  (
    [],
    [
      (U.as_untyped("zip"), 3 |> U.int_prim)
      |> Statement.of_variable
      |> U.as_nil,
      (U.as_untyped("zap"), 4 |> U.int_prim)
      |> Statement.of_variable
      |> U.as_nil,
      (
        "zip" |> Expression.of_identifier |> U.as_int,
        "zap" |> Expression.of_identifier |> U.as_int,
      )
      |> Expression.of_multiply_op
      |> U.as_int
      |> Statement.of_effect
      |> U.as_int,
    ]
    |> Expression.of_closure
    |> U.as_int,
  )
  |> Declaration.of_function,
);

let __inline_view = (
  "foo",
  (
    [
      (U.as_untyped("bar"), None, None) |> U.as_int,
      (U.as_untyped("fizz"), None, Some(3 |> U.int_prim)) |> U.as_int,
    ],
    [],
    [
      (
        "bar" |> Expression.of_identifier |> U.as_int,
        "fizz" |> Expression.of_identifier |> U.as_int,
      )
      |> Expression.of_add_op
      |> U.as_int
      |> KSX.Child.of_inline
      |> U.as_untyped,
    ]
    |> KSX.of_fragment
    |> Expression.of_ksx
    |> U.as_element,
  )
  |> Declaration.of_view,
);

let __multiline_view = (
  "buzz",
  (
    [],
    [],
    [
      (U.as_untyped("zip"), 3 |> U.int_prim)
      |> Statement.of_variable
      |> U.as_nil,
      (U.as_untyped("zap"), 4 |> U.int_prim)
      |> Statement.of_variable
      |> U.as_nil,
      [
        (
          "zip" |> Expression.of_identifier |> U.as_int,
          "zap" |> Expression.of_identifier |> U.as_int,
        )
        |> Expression.of_multiply_op
        |> U.as_int
        |> KSX.Child.of_inline
        |> U.as_untyped,
      ]
      |> KSX.of_fragment
      |> Expression.of_ksx
      |> U.as_element
      |> Statement.of_effect
      |> U.as_element,
    ]
    |> Expression.of_closure
    |> U.as_int,
  )
  |> Declaration.of_view,
);

let _assert_declaration = (expected, actual) =>
  Assert.string(expected, actual |> ~@Fmt.root(Declaration.format));
let _assert_declaration_list = (expected, actual) =>
  Assert.string(
    expected,
    actual |> ~@Fmt.root(Language.Formatter.format_declaration_list),
  );

let suite =
  "Grammar.Formatter | Declaration"
  >::: [
    "pp_declaration() - constant"
    >: (
      () =>
        _assert_declaration(
          "const foo = nil;",
          ("foo", Declaration.of_constant(U.nil_prim)),
        )
    ),
    "pp_declaration() - empty enumerated"
    >: (
      () =>
        _assert_declaration(
          "enum Account = | ;",
          ("Account", Declaration.of_enumerated([])),
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
            "Digits",
            Declaration.of_enumerated([
              (U.as_untyped("Zero"), []),
              (U.as_untyped("One"), []),
              (U.as_untyped("Two"), []),
              (U.as_untyped("Three"), []),
              (U.as_untyped("Four"), []),
              (U.as_untyped("Five"), []),
              (U.as_untyped("Six"), []),
              (U.as_untyped("Seven"), []),
              (U.as_untyped("Eight"), []),
              (U.as_untyped("Nine"), []),
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
