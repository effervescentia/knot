open Kore;

module A = AST.Result;
module Formatter = Language.Formatter;
module T = AST.Type;
module U = Util.ResultUtil;

let _assert_expression = (expected, actual) =>
  Assert.string(expected, actual |> ~@Fmt.root(KExpression.Plugin.pp));

let suite =
  "Grammar.Formatter | Expression"
  >::: [
    "primitive" >: (() => _assert_expression("nil", A.of_prim(A.nil))),
    "identifier" >: (() => _assert_expression("fooBar", A.of_id("fooBar"))),
    "group"
    >: (() => _assert_expression("123", 123 |> U.int_prim |> A.of_group)),
    "nested group"
    >: (
      () =>
        _assert_expression(
          "123",
          123 |> U.int_prim |> A.of_group |> U.as_int |> A.of_group,
        )
    ),
    "dot access"
    >: (
      () =>
        _assert_expression(
          "foo.bar",
          (
            "foo"
            |> A.of_id
            |> U.as_struct([("bar", (T.Valid(`Boolean), true))]),
            U.as_untyped("bar"),
          )
          |> A.of_dot_access,
        )
    ),
    "function call"
    >: (
      () =>
        _assert_expression(
          "foo(bar)",
          (
            "foo"
            |> A.of_id
            |> U.as_function([T.Valid(`String)], T.Valid(`Boolean)),
            ["bar" |> A.of_id |> U.as_string],
          )
          |> A.of_func_call,
        )
    ),
    "grouped binary operation"
    >: (
      () =>
        _assert_expression(
          "(123 + 456)",
          (123 |> U.int_prim, 456 |> U.int_prim)
          |> A.of_add_op
          |> U.as_int
          |> A.of_group,
        )
    ),
    "nested grouped binary operation"
    >: (
      () =>
        _assert_expression(
          "(123 + 456)",
          (123 |> U.int_prim, 456 |> U.int_prim)
          |> A.of_add_op
          |> U.as_int
          |> A.of_group
          |> U.as_int
          |> A.of_group,
        )
    ),
    "closure"
    >: (
      () =>
        _assert_expression(
          "{
  true;
  false;
  nil;
}",
          [
            true |> U.bool_prim |> A.of_expr |> U.as_bool,
            false |> U.bool_prim |> A.of_expr |> U.as_bool,
            U.nil_prim |> A.of_expr |> U.as_nil,
          ]
          |> A.of_closure,
        )
    ),
    "unary operation"
    >: (() => _assert_expression("!true", true |> U.bool_prim |> A.of_not_op)),
    "binary operation"
    >: (
      () =>
        _assert_expression(
          "true || false",
          (true |> U.bool_prim, false |> U.bool_prim) |> A.of_or_op,
        )
    ),
    "style binding"
    >: (
      () =>
        _assert_expression(
          "foo::bar",
          (
            "foo" |> A.of_id |> U.as_view([], Valid(`Nil)),
            "bar" |> A.of_id |> U.as_style,
          )
          |> A.of_bind_style,
        )
    ),
    "style literal binding"
    >: (
      () =>
        _assert_expression(
          "foo::{
  color: $red,
}",
          (
            "foo" |> A.of_id |> U.as_view([], Valid(`Nil)),
            [
              (
                "color" |> U.as_function([], T.Valid(`Nil)),
                "$red" |> A.of_id |> U.as_string,
              )
              |> U.as_untyped,
            ]
            |> A.of_style
            |> U.as_style,
          )
          |> A.of_bind_style,
        )
    ),
    "JSX"
    >: (
      () =>
        _assert_expression(
          "<Foo>
  bar
</Foo>",
          (
            "Foo" |> U.as_view([], Valid(`Nil)),
            [],
            ["bar" |> A.of_text |> U.as_untyped],
          )
          |> A.of_tag
          |> A.of_jsx,
        )
    ),
    "style"
    >: (
      () =>
        _assert_expression(
          "style {
  height: $px(20),
  color: $red,
}",
          [
            (
              "height" |> U.as_function([], T.Valid(`Nil)),
              ("$px" |> A.of_id |> U.as_string, [20 |> U.int_prim])
              |> A.of_func_call
              |> U.as_string,
            )
            |> U.as_untyped,
            (
              "color" |> U.as_function([], T.Valid(`Nil)),
              "$red" |> A.of_id |> U.as_string,
            )
            |> U.as_untyped,
          ]
          |> A.of_style,
        )
    ),
  ];
