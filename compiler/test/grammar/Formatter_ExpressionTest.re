open Kore;

module Formatter = Language.Formatter;
module U = Util.ResultUtil;

let _assert_expression = (expected, actual) =>
  Assert.string(expected, actual |> ~@Fmt.root(KExpression.Plugin.format));

let suite =
  "Grammar.Formatter | Expression"
  >::: [
    "primitive"
    >: (
      () => _assert_expression("nil", Expression.of_primitive(Primitive.nil))
    ),
    "identifier"
    >: (
      () => _assert_expression("fooBar", Expression.of_identifier("fooBar"))
    ),
    "group"
    >: (
      () =>
        _assert_expression("123", 123 |> U.int_prim |> Expression.of_group)
    ),
    "nested group"
    >: (
      () =>
        _assert_expression(
          "123",
          123
          |> U.int_prim
          |> Expression.of_group
          |> U.as_int
          |> Expression.of_group,
        )
    ),
    "dot access"
    >: (
      () =>
        _assert_expression(
          "foo.bar",
          (
            "foo"
            |> Expression.of_identifier
            |> U.as_struct([("bar", (Valid(Boolean), true))]),
            U.as_untyped("bar"),
          )
          |> Expression.of_dot_access,
        )
    ),
    "function call"
    >: (
      () =>
        _assert_expression(
          "foo(bar)",
          (
            "foo"
            |> Expression.of_identifier
            |> U.as_function([Valid(String)], Valid(Boolean)),
            ["bar" |> Expression.of_identifier |> U.as_string],
          )
          |> Expression.of_function_call,
        )
    ),
    "grouped binary operation"
    >: (
      () =>
        _assert_expression(
          "(123 + 456)",
          (123 |> U.int_prim, 456 |> U.int_prim)
          |> Expression.of_add_op
          |> U.as_int
          |> Expression.of_group,
        )
    ),
    "nested grouped binary operation"
    >: (
      () =>
        _assert_expression(
          "(123 + 456)",
          (123 |> U.int_prim, 456 |> U.int_prim)
          |> Expression.of_add_op
          |> U.as_int
          |> Expression.of_group
          |> U.as_int
          |> Expression.of_group,
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
            true |> U.bool_prim |> Statement.of_effect |> U.as_bool,
            false |> U.bool_prim |> Statement.of_effect |> U.as_bool,
            U.nil_prim |> Statement.of_effect |> U.as_nil,
          ]
          |> Expression.of_closure,
        )
    ),
    "unary operation"
    >: (
      () =>
        _assert_expression(
          "!true",
          true |> U.bool_prim |> Expression.of_not_op,
        )
    ),
    "binary operation"
    >: (
      () =>
        _assert_expression(
          "true || false",
          (true |> U.bool_prim, false |> U.bool_prim) |> Expression.of_or_op,
        )
    ),
    "style binding"
    >: (
      () =>
        _assert_expression(
          "foo::bar",
          (
            KSX.ViewKind.Component,
            "foo" |> Expression.of_identifier |> U.as_view([], Valid(Nil)),
            "bar" |> Expression.of_identifier |> U.as_style,
          )
          |> Expression.of_bind_style,
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
            KSX.ViewKind.Component,
            "foo" |> Expression.of_identifier |> U.as_view([], Valid(Nil)),
            [
              (
                "color" |> U.as_function([], Valid(Nil)),
                "$red" |> Expression.of_identifier |> U.as_string,
              )
              |> U.as_untyped,
            ]
            |> Expression.of_style
            |> U.as_style,
          )
          |> Expression.of_bind_style,
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
            "Foo" |> U.as_view([], Valid(Nil)),
            [],
            [],
            ["bar" |> KSX.Child.of_text |> U.as_untyped],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx,
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
              "height" |> U.as_function([], Valid(Nil)),
              (
                "$px" |> Expression.of_identifier |> U.as_string,
                [20 |> U.int_prim],
              )
              |> Expression.of_function_call
              |> U.as_string,
            )
            |> U.as_untyped,
            (
              "color" |> U.as_function([], Valid(Nil)),
              "$red" |> Expression.of_identifier |> U.as_string,
            )
            |> U.as_untyped,
          ]
          |> Expression.of_style,
        )
    ),
  ];
