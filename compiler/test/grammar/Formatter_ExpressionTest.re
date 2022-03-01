open Kore;

module Formatter = Grammar.Formatter;
module U = Util.ResultUtilV2;

let _assert_expression = (expected, actual) =>
  Assert.string(expected, actual |> ~@Fmt.root(pp_expression));

let suite =
  "Grammar.Formatter | Expression"
  >::: [
    "primitive" >: (() => _assert_expression("nil", A.nil |> A.of_prim)),
    "identifier"
    >: (
      () => _assert_expression("fooBar", "fooBar" |> A.of_public |> A.of_id)
    ),
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
    "JSX"
    >: (
      () =>
        _assert_expression(
          "<Foo>
  bar
</Foo>",
          (
            "Foo" |> A.of_public |> U.as_raw_node,
            [],
            ["bar" |> A.of_text |> U.as_raw_node],
          )
          |> A.of_tag
          |> A.of_jsx,
        )
    ),
  ];
