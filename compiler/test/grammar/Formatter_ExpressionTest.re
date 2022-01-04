open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Formatter = Grammar.Formatter;

let _assert_expression = (expected, actual) =>
  Assert.string(expected, actual |> ~@Fmt.root(pp_expression));

let suite =
  "Grammar.Formatter | Expression"
  >::: [
    "primitive" >: (() => _assert_expression("nil", nil |> as_nil |> of_prim)),
    "identifier"
    >: (
      () =>
        _assert_expression(
          "fooBar",
          "fooBar" |> of_public |> as_generic(0, 0) |> of_id,
        )
    ),
    "group" >: (() => _assert_expression("123", 123 |> int_prim |> of_group)),
    "nested group"
    >: (
      () =>
        _assert_expression(
          "123",
          123 |> int_prim |> of_group |> as_int |> of_group,
        )
    ),
    "grouped binary operation"
    >: (
      () =>
        _assert_expression(
          "(123 + 456)",
          (123 |> int_prim, 456 |> int_prim)
          |> of_add_op
          |> as_int
          |> of_group,
        )
    ),
    "nested grouped binary operation"
    >: (
      () =>
        _assert_expression(
          "(123 + 456)",
          (123 |> int_prim, 456 |> int_prim)
          |> of_add_op
          |> as_int
          |> of_group
          |> as_int
          |> of_group,
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
            true |> bool_prim |> of_expr |> as_bool,
            false |> bool_prim |> of_expr |> as_bool,
            nil_prim |> of_expr |> as_nil,
          ]
          |> of_closure,
        )
    ),
    "unary operation"
    >: (() => _assert_expression("!true", true |> bool_prim |> of_not_op)),
    "binary operation"
    >: (
      () =>
        _assert_expression(
          "true || false",
          (true |> bool_prim, false |> bool_prim) |> of_or_op,
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
            "Foo" |> of_public |> as_raw_node,
            [],
            ["bar" |> as_string |> of_text |> as_string],
          )
          |> of_tag
          |> as_element
          |> of_jsx,
        )
    ),
  ];
