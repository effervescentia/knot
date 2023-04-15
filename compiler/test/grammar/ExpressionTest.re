open Kore;

module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = Expression.node_t(unit);

    let parser =
      KExpression.Plugin.parse % Assert.parse_completely % Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              KExpression.Plugin.to_xml(_ => "Unknown") % Fmt.xml_string(ppf),
            (==),
          ),
          "program matches",
        )
      );
  });

let _generate_spaced_identifier_ops =
  List.map(((tag, op)) =>
    (
      (
        (
          "a" |> Expression.of_identifier |> U.as_node,
          "b" |> Expression.of_identifier |> U.as_node,
        )
        |> tag,
        "c" |> Expression.of_identifier |> U.as_node,
      )
      |> tag,
      Fmt.str("a %s b %s c", op, op),
    )
  );
let _generate_spaced_bool_ops = (~lhs=true, ~rhs=false) =>
  List.map(((tag, op)) =>
    [
      (
        (U.bool_prim(lhs), U.bool_prim(rhs)) |> tag |> U.as_node,
        Fmt.str("%b%s%b", lhs, op, rhs),
      ),
      (
        (U.bool_prim(lhs), U.bool_prim(rhs)) |> tag |> U.as_node,
        Fmt.str(" %b %s %b ", lhs, op, rhs),
      ),
    ]
  );
let _generate_spaced_int_ops = (~lhs=123, ~rhs=456) =>
  List.map(((tag, op)) =>
    [
      (
        (U.int_prim(lhs), U.int_prim(rhs)) |> tag,
        Fmt.str("%d%s%d", lhs, op, rhs),
      ),
      (
        (U.int_prim(lhs), U.int_prim(rhs)) |> tag,
        Fmt.str(" %d %s %d ", lhs, op, rhs),
      ),
    ]
  );

let _assert_parse_many = (~report=AST.Error.throw) =>
  List.iter(Tuple.join2(Assert.parse(~report)));

let suite =
  "Grammar.Expression"
  >::: [
    "no parse" >: (() => Assert.no_parse("~gibberish")),
    "parse primitive" >: (() => Assert.parse(U.int_prim(123), "123")),
    "parse identifier"
    >: (
      () =>
        Assert.parse("foo" |> Expression.of_identifier |> U.as_node, "foo")
    ),
    "parse group"
    >: (
      () =>
        Assert.parse(
          "foo"
          |> Expression.of_identifier
          |> U.as_node
          |> Expression.of_group
          |> U.as_node,
          "(foo)",
        )
    ),
    "parse closure"
    >: (
      () =>
        Assert.parse(
          [
            "foo"
            |> Expression.of_identifier
            |> U.as_node
            |> Statement.of_effect
            |> U.as_node,
            (U.as_untyped("x"), U.bool_prim(false))
            |> Statement.of_variable
            |> U.as_node,
            (
              U.as_untyped("y"),
              "foo" |> Expression.of_identifier |> U.as_node,
            )
            |> Statement.of_variable
            |> U.as_node,
            (U.as_untyped("z"), "y" |> Expression.of_identifier |> U.as_node)
            |> Statement.of_variable
            |> U.as_node,
            (U.int_prim(1), U.int_prim(2))
            |> Expression.of_add_op
            |> U.as_node
            |> Statement.of_effect
            |> U.as_node,
          ]
          |> Expression.of_closure
          |> U.as_node,
          "{
            foo;
            let x = false;
            let y = foo;
            let z = y;
            1 + 2;
          }",
        )
    ),
    "parse dot access - identifier root"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> Expression.of_identifier |> U.as_node,
            U.as_untyped("bar"),
          )
          |> Expression.of_dot_access
          |> U.as_node,
          "foo.bar",
        )
    ),
    "parse dot access - group root"
    >: (
      () =>
        Assert.parse(
          (
            "foo"
            |> Expression.of_identifier
            |> U.as_node
            |> Expression.of_group
            |> U.as_node,
            U.as_untyped("bar"),
          )
          |> Expression.of_dot_access
          |> U.as_node,
          "(foo).bar",
        )
    ),
    "parse dot access - closure root"
    >: (
      () =>
        Assert.parse(
          (
            [
              "foo"
              |> Expression.of_identifier
              |> U.as_node
              |> Statement.of_effect
              |> U.as_node,
            ]
            |> Expression.of_closure
            |> U.as_node,
            U.as_untyped("bar"),
          )
          |> Expression.of_dot_access
          |> U.as_node,
          "{ foo; }.bar",
        )
    ),
    "parse style expression"
    >: (
      () =>
        Assert.parse(
          [
            (
              "color" |> U.as_node,
              "$pink" |> Expression.of_identifier |> U.as_node,
            )
            |> U.as_node,
            ("height" |> U.as_node, U.string_prim("20px")) |> U.as_node,
          ]
          |> Expression.of_style
          |> U.as_node,
          "style {
  color: $pink,
  height: \"20px\",
}",
        )
    ),
    "parse style binding - identifier root"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> Expression.of_identifier |> U.as_node,
            "bar" |> Expression.of_identifier |> U.as_node,
          )
          |> Expression.of_bind_element_style
          |> U.as_node,
          "foo::bar",
        )
    ),
    "parse style binding - empty"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> Expression.of_identifier |> U.as_node,
            [] |> Expression.of_style |> U.as_node,
          )
          |> Expression.of_bind_element_style
          |> U.as_node,
          "foo::{}",
        )
    ),
    "parse style binding - literal"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> Expression.of_identifier |> U.as_node,
            [
              (
                "color" |> U.as_node,
                "$pink" |> Expression.of_identifier |> U.as_node,
              )
              |> U.as_node,
              ("height" |> U.as_node, U.string_prim("20px")) |> U.as_node,
              (
                "width" |> U.as_node,
                (
                  "$px" |> Expression.of_identifier |> U.as_node,
                  [U.float_prim((22.5, 3))],
                )
                |> Expression.of_function_call
                |> U.as_node,
              )
              |> U.as_node,
            ]
            |> Expression.of_style
            |> U.as_node,
          )
          |> Expression.of_bind_element_style
          |> U.as_node,
          "foo::{
  color: $pink,
  height: \"20px\",
  width: $px(22.5),
}",
        )
    ),
    "parse function call - identifier root"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> Expression.of_identifier |> U.as_node,
            ["bar" |> Expression.of_identifier |> U.as_node],
          )
          |> Expression.of_function_call
          |> U.as_node,
          "foo(bar)",
        )
    ),
    "parse function call - group root"
    >: (
      () =>
        Assert.parse(
          (
            "foo"
            |> Expression.of_identifier
            |> U.as_node
            |> Expression.of_group
            |> U.as_node,
            ["bar" |> Expression.of_identifier |> U.as_node],
          )
          |> Expression.of_function_call
          |> U.as_node,
          "(foo)(bar)",
        )
    ),
    "parse function call - closure root"
    >: (
      () =>
        Assert.parse(
          (
            [
              "foo"
              |> Expression.of_identifier
              |> U.as_node
              |> Statement.of_effect
              |> U.as_node,
            ]
            |> Expression.of_closure
            |> U.as_node,
            ["bar" |> Expression.of_identifier |> U.as_node],
          )
          |> Expression.of_function_call
          |> U.as_node,
          "{ foo; }(bar)",
        )
    ),
    "parse function call - dot access root"
    >: (
      () =>
        Assert.parse(
          (
            (
              "foo" |> Expression.of_identifier |> U.as_node,
              U.as_untyped("bar"),
            )
            |> Expression.of_dot_access
            |> U.as_node,
            ["fizz" |> Expression.of_identifier |> U.as_node],
          )
          |> Expression.of_function_call
          |> U.as_node,
          "foo.bar(fizz)",
        )
    ),
    "parse unary - negative"
    >: (
      () =>
        Assert.parse(
          123 |> U.int_prim |> Expression.of_negative_op |> U.as_node,
          "-123",
        )
    ),
    "parse unary - logical not"
    >: (
      () =>
        Assert.parse(
          true |> U.bool_prim |> Expression.of_not_op |> U.as_node,
          "!true",
        )
    ),
    "parse boolean logic"
    >: (
      () =>
        _assert_parse_many(
          [(Expression.of_and_op, "&&"), (Expression.of_or_op, "||")]
          |> _generate_spaced_bool_ops
          |> List.flatten,
        )
    ),
    "parse arithmetic"
    >: (
      () =>
        _assert_parse_many(
          [
            (Expression.of_add_op % U.as_node, "+"),
            (Expression.of_subtract_op % U.as_node, "-"),
            (Expression.of_multiply_op % U.as_node, "*"),
            (Expression.of_divide_op % U.as_node, "/"),
          ]
          |> _generate_spaced_int_ops
          |> List.flatten,
        )
    ),
    "parse comparison"
    >: (
      () =>
        _assert_parse_many(
          [
            (Expression.of_lte_op % U.as_node, "<="),
            (Expression.of_lt_op % U.as_node, "<"),
            (Expression.of_gte_op % U.as_node, ">="),
            (Expression.of_gt_op % U.as_node, ">"),
          ]
          |> _generate_spaced_int_ops
          |> List.flatten,
        )
    ),
    "parse complex expression"
    >: (
      () =>
        _assert_parse_many([
          (
            (
              (
                U.int_prim(2),
                (
                  U.int_prim(3),
                  (U.int_prim(4), U.int_prim(5))
                  |> Expression.of_exponent_op
                  |> U.as_node,
                )
                |> Expression.of_multiply_op
                |> U.as_node,
              )
              |> Expression.of_add_op
              |> U.as_node,
              (
                6 |> U.int_prim |> Expression.of_negative_op |> U.as_node,
                U.int_prim(7),
              )
              |> Expression.of_divide_op
              |> U.as_node,
            )
            |> Expression.of_subtract_op
            |> U.as_node,
            "2 + 3 * 4 ^ 5 - -6 / 7",
          ),
          (
            (
              (U.int_prim(2), U.int_prim(3))
              |> Expression.of_add_op
              |> U.as_node
              |> Expression.of_group
              |> U.as_node,
              (
                U.int_prim(4),
                (
                  U.int_prim(5),
                  (U.int_prim(6), U.int_prim(7))
                  |> Expression.of_divide_op
                  |> U.as_node
                  |> Expression.of_group
                  |> U.as_node
                  |> Expression.of_negative_op
                  |> U.as_node,
                )
                |> Expression.of_subtract_op
                |> U.as_node
                |> Expression.of_group
                |> U.as_node,
              )
              |> Expression.of_exponent_op
              |> U.as_node,
            )
            |> Expression.of_multiply_op
            |> U.as_node,
            "(2 + 3) * 4 ^ (5 - -(6 / 7))",
          ),
          (
            (
              (
                "a" |> Expression.of_identifier |> U.as_node,
                (
                  (
                    "b" |> Expression.of_identifier |> U.as_node,
                    "c" |> Expression.of_identifier |> U.as_node,
                  )
                  |> Expression.of_gt_op
                  |> U.as_node,
                  (
                    "e" |> Expression.of_identifier |> U.as_node,
                    "f" |> Expression.of_identifier |> U.as_node,
                  )
                  |> Expression.of_lte_op
                  |> U.as_node,
                )
                |> Expression.of_or_op
                |> U.as_node
                |> Expression.of_group
                |> U.as_node,
              )
              |> Expression.of_and_op
              |> U.as_node,
              (
                "g" |> Expression.of_identifier |> U.as_node,
                "h" |> Expression.of_identifier |> U.as_node,
              )
              |> Expression.of_or_op
              |> U.as_node
              |> Expression.of_group
              |> U.as_node
              |> Expression.of_not_op
              |> U.as_node
              |> Expression.of_group
              |> U.as_node,
            )
            |> Expression.of_and_op
            |> U.as_node,
            "a && (b > c || e <= f) && (!(g || h))",
          ),
        ])
    ),
    "parse left-associative - add, subtract, multiply"
    >: (
      () =>
        _assert_parse_many(
          [
            (Expression.of_add_op % U.as_node, "+"),
            (Expression.of_subtract_op % U.as_node, "-"),
            (Expression.of_multiply_op % U.as_node, "*"),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - divide"
    >: (
      () =>
        _assert_parse_many(
          [(Expression.of_divide_op % U.as_node, "/")]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - logical and, logical or"
    >: (
      () =>
        _assert_parse_many(
          [
            (Expression.of_and_op % U.as_node, "&&"),
            (Expression.of_or_op % U.as_node, "||"),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - comparison"
    >: (
      () =>
        _assert_parse_many(
          [
            (Expression.of_lte_op % U.as_node, "<="),
            (Expression.of_lt_op % U.as_node, "<"),
            (Expression.of_gte_op % U.as_node, ">="),
            (Expression.of_gt_op % U.as_node, ">"),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - equality"
    >: (
      () =>
        _assert_parse_many(
          [
            (Expression.of_equal_op % U.as_node, "=="),
            (Expression.of_unequal_op % U.as_node, "!="),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - dot access"
    >: (
      () =>
        Assert.parse(
          (
            (
              (
                "a" |> Expression.of_identifier |> U.as_node,
                U.as_untyped("b"),
              )
              |> Expression.of_dot_access
              |> U.as_node,
              U.as_untyped("c"),
            )
            |> Expression.of_dot_access
            |> U.as_node,
            U.as_untyped("d"),
          )
          |> Expression.of_dot_access
          |> U.as_node,
          "a.b.c.d",
        )
    ),
    "parse left-associative - style binding"
    >: (
      () =>
        Assert.parse(
          (
            (
              "a" |> Expression.of_identifier |> U.as_node,
              "b" |> Expression.of_identifier |> U.as_node,
            )
            |> Expression.of_bind_element_style
            |> U.as_node,
            "c" |> Expression.of_identifier |> U.as_node,
          )
          |> Expression.of_bind_element_style
          |> U.as_node,
          "a::b::c",
        )
    ),
    "parse right-associative - exponent"
    >: (
      () =>
        Assert.parse(
          (
            "a" |> Expression.of_identifier |> U.as_node,
            (
              "b" |> Expression.of_identifier |> U.as_node,
              "c" |> Expression.of_identifier |> U.as_node,
            )
            |> Expression.of_exponent_op
            |> U.as_node,
          )
          |> Expression.of_exponent_op
          |> U.as_node,
          "a ^ b ^ c",
        )
    ),
    "parse right-associative - negative"
    >: (
      () =>
        Assert.parse(
          "a"
          |> Expression.of_identifier
          |> U.as_node
          |> Expression.of_negative_op
          |> U.as_node
          |> Expression.of_negative_op
          |> U.as_node
          |> Expression.of_negative_op
          |> U.as_node,
          "- - - a",
        )
    ),
    "parse right-associative - logical not"
    >: (
      () =>
        Assert.parse(
          "a"
          |> Expression.of_identifier
          |> U.as_node
          |> Expression.of_not_op
          |> U.as_node
          |> Expression.of_not_op
          |> U.as_node
          |> Expression.of_not_op
          |> U.as_node,
          "! ! ! a",
        )
    ),
  ];
