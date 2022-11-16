open Kore;

module AR = AST.Raw;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = AR.expression_t;

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
        ("a" |> AR.of_id |> U.as_node, "b" |> AR.of_id |> U.as_node) |> tag,
        "c" |> AR.of_id |> U.as_node,
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
    >: (() => Assert.parse("foo" |> AR.of_id |> U.as_node, "foo")),
    "parse group"
    >: (
      () =>
        Assert.parse(
          "foo" |> AR.of_id |> U.as_node |> AR.of_group |> U.as_node,
          "(foo)",
        )
    ),
    "parse closure"
    >: (
      () =>
        Assert.parse(
          [
            "foo" |> AR.of_id |> U.as_node |> AR.of_expr |> U.as_node,
            (U.as_untyped("x"), U.bool_prim(false))
            |> AR.of_var
            |> U.as_node,
            (U.as_untyped("y"), "foo" |> AR.of_id |> U.as_node)
            |> AR.of_var
            |> U.as_node,
            (U.as_untyped("z"), "y" |> AR.of_id |> U.as_node)
            |> AR.of_var
            |> U.as_node,
            (U.int_prim(1), U.int_prim(2))
            |> AR.of_add_op
            |> U.as_node
            |> AR.of_expr
            |> U.as_node,
          ]
          |> AR.of_closure
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
          ("foo" |> AR.of_id |> U.as_node, U.as_untyped("bar"))
          |> AR.of_dot_access
          |> U.as_node,
          "foo.bar",
        )
    ),
    "parse dot access - group root"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> AR.of_id |> U.as_node |> AR.of_group |> U.as_node,
            U.as_untyped("bar"),
          )
          |> AR.of_dot_access
          |> U.as_node,
          "(foo).bar",
        )
    ),
    "parse dot access - closure root"
    >: (
      () =>
        Assert.parse(
          (
            ["foo" |> AR.of_id |> U.as_node |> AR.of_expr |> U.as_node]
            |> AR.of_closure
            |> U.as_node,
            U.as_untyped("bar"),
          )
          |> AR.of_dot_access
          |> U.as_node,
          "{ foo; }.bar",
        )
    ),
    "parse style expression"
    >: (
      () =>
        Assert.parse(
          [
            ("color" |> U.as_node, "$pink" |> AR.of_id |> U.as_node)
            |> U.as_node,
            ("height" |> U.as_node, U.string_prim("20px")) |> U.as_node,
          ]
          |> AR.of_style
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
          ("foo" |> AR.of_id |> U.as_node, "bar" |> AR.of_id |> U.as_node)
          |> AR.of_bind_style
          |> U.as_node,
          "foo::bar",
        )
    ),
    "parse style binding - literal"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> AR.of_id |> U.as_node,
            [
              ("color" |> U.as_node, "$pink" |> AR.of_id |> U.as_node)
              |> U.as_node,
              ("height" |> U.as_node, U.string_prim("20px")) |> U.as_node,
            ]
            |> AR.of_style
            |> U.as_node,
          )
          |> AR.of_bind_style
          |> U.as_node,
          "foo::{
  color: $pink,
  height: \"20px\",
}",
        )
    ),
    "parse function call - identifier root"
    >: (
      () =>
        Assert.parse(
          ("foo" |> AR.of_id |> U.as_node, ["bar" |> AR.of_id |> U.as_node])
          |> AR.of_func_call
          |> U.as_node,
          "foo(bar)",
        )
    ),
    "parse function call - group root"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> AR.of_id |> U.as_node |> AR.of_group |> U.as_node,
            ["bar" |> AR.of_id |> U.as_node],
          )
          |> AR.of_func_call
          |> U.as_node,
          "(foo)(bar)",
        )
    ),
    "parse function call - closure root"
    >: (
      () =>
        Assert.parse(
          (
            ["foo" |> AR.of_id |> U.as_node |> AR.of_expr |> U.as_node]
            |> AR.of_closure
            |> U.as_node,
            ["bar" |> AR.of_id |> U.as_node],
          )
          |> AR.of_func_call
          |> U.as_node,
          "{ foo; }(bar)",
        )
    ),
    "parse function call - dot access root"
    >: (
      () =>
        Assert.parse(
          (
            ("foo" |> AR.of_id |> U.as_node, U.as_untyped("bar"))
            |> AR.of_dot_access
            |> U.as_node,
            ["fizz" |> AR.of_id |> U.as_node],
          )
          |> AR.of_func_call
          |> U.as_node,
          "foo.bar(fizz)",
        )
    ),
    "parse unary - negative"
    >: (
      () =>
        Assert.parse(123 |> U.int_prim |> AR.of_neg_op |> U.as_node, "-123")
    ),
    "parse unary - logical not"
    >: (
      () =>
        Assert.parse(
          true |> U.bool_prim |> AR.of_not_op |> U.as_node,
          "!true",
        )
    ),
    "parse boolean logic"
    >: (
      () =>
        _assert_parse_many(
          [(AR.of_and_op, "&&"), (AR.of_or_op, "||")]
          |> _generate_spaced_bool_ops
          |> List.flatten,
        )
    ),
    "parse arithmetic"
    >: (
      () =>
        _assert_parse_many(
          [
            (AR.of_add_op % U.as_node, "+"),
            (AR.of_sub_op % U.as_node, "-"),
            (AR.of_mult_op % U.as_node, "*"),
            (AR.of_div_op % U.as_node, "/"),
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
            (AR.of_lte_op % U.as_node, "<="),
            (AR.of_lt_op % U.as_node, "<"),
            (AR.of_gte_op % U.as_node, ">="),
            (AR.of_gt_op % U.as_node, ">"),
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
                  |> AR.of_expo_op
                  |> U.as_node,
                )
                |> AR.of_mult_op
                |> U.as_node,
              )
              |> AR.of_add_op
              |> U.as_node,
              (6 |> U.int_prim |> AR.of_neg_op |> U.as_node, U.int_prim(7))
              |> AR.of_div_op
              |> U.as_node,
            )
            |> AR.of_sub_op
            |> U.as_node,
            "2 + 3 * 4 ^ 5 - -6 / 7",
          ),
          (
            (
              (U.int_prim(2), U.int_prim(3))
              |> AR.of_add_op
              |> U.as_node
              |> AR.of_group
              |> U.as_node,
              (
                U.int_prim(4),
                (
                  U.int_prim(5),
                  (U.int_prim(6), U.int_prim(7))
                  |> AR.of_div_op
                  |> U.as_node
                  |> AR.of_group
                  |> U.as_node
                  |> AR.of_neg_op
                  |> U.as_node,
                )
                |> AR.of_sub_op
                |> U.as_node
                |> AR.of_group
                |> U.as_node,
              )
              |> AR.of_expo_op
              |> U.as_node,
            )
            |> AR.of_mult_op
            |> U.as_node,
            "(2 + 3) * 4 ^ (5 - -(6 / 7))",
          ),
          (
            (
              (
                "a" |> AR.of_id |> U.as_node,
                (
                  ("b" |> AR.of_id |> U.as_node, "c" |> AR.of_id |> U.as_node)
                  |> AR.of_gt_op
                  |> U.as_node,
                  ("e" |> AR.of_id |> U.as_node, "f" |> AR.of_id |> U.as_node)
                  |> AR.of_lte_op
                  |> U.as_node,
                )
                |> AR.of_or_op
                |> U.as_node
                |> AR.of_group
                |> U.as_node,
              )
              |> AR.of_and_op
              |> U.as_node,
              ("g" |> AR.of_id |> U.as_node, "h" |> AR.of_id |> U.as_node)
              |> AR.of_or_op
              |> U.as_node
              |> AR.of_group
              |> U.as_node
              |> AR.of_not_op
              |> U.as_node
              |> AR.of_group
              |> U.as_node,
            )
            |> AR.of_and_op
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
            (AR.of_add_op % U.as_node, "+"),
            (AR.of_sub_op % U.as_node, "-"),
            (AR.of_mult_op % U.as_node, "*"),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - divide"
    >: (
      () =>
        _assert_parse_many(
          [(AR.of_div_op % U.as_node, "/")]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - logical and, logical or"
    >: (
      () =>
        _assert_parse_many(
          [
            (AR.of_and_op % U.as_node, "&&"),
            (AR.of_or_op % U.as_node, "||"),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - comparison"
    >: (
      () =>
        _assert_parse_many(
          [
            (AR.of_lte_op % U.as_node, "<="),
            (AR.of_lt_op % U.as_node, "<"),
            (AR.of_gte_op % U.as_node, ">="),
            (AR.of_gt_op % U.as_node, ">"),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - equality"
    >: (
      () =>
        _assert_parse_many(
          [
            (AR.of_eq_op % U.as_node, "=="),
            (AR.of_ineq_op % U.as_node, "!="),
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
              ("a" |> AR.of_id |> U.as_node, U.as_untyped("b"))
              |> AR.of_dot_access
              |> U.as_node,
              U.as_untyped("c"),
            )
            |> AR.of_dot_access
            |> U.as_node,
            U.as_untyped("d"),
          )
          |> AR.of_dot_access
          |> U.as_node,
          "a.b.c.d",
        )
    ),
    "parse left-associative - style binding"
    >: (
      () =>
        Assert.parse(
          (
            ("a" |> AR.of_id |> U.as_node, "b" |> AR.of_id |> U.as_node)
            |> AR.of_bind_style
            |> U.as_node,
            "c" |> AR.of_id |> U.as_node,
          )
          |> AR.of_bind_style
          |> U.as_node,
          "a::b::c",
        )
    ),
    "parse right-associative - exponent"
    >: (
      () =>
        Assert.parse(
          (
            "a" |> AR.of_id |> U.as_node,
            ("b" |> AR.of_id |> U.as_node, "c" |> AR.of_id |> U.as_node)
            |> AR.of_expo_op
            |> U.as_node,
          )
          |> AR.of_expo_op
          |> U.as_node,
          "a ^ b ^ c",
        )
    ),
    "parse right-associative - negative"
    >: (
      () =>
        Assert.parse(
          "a"
          |> AR.of_id
          |> U.as_node
          |> AR.of_neg_op
          |> U.as_node
          |> AR.of_neg_op
          |> U.as_node
          |> AR.of_neg_op
          |> U.as_node,
          "- - - a",
        )
    ),
    "parse right-associative - logical not"
    >: (
      () =>
        Assert.parse(
          "a"
          |> AR.of_id
          |> U.as_node
          |> AR.of_not_op
          |> U.as_node
          |> AR.of_not_op
          |> U.as_node
          |> AR.of_not_op
          |> U.as_node,
          "! ! ! a",
        )
    ),
  ];
