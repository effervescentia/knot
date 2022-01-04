open Kore;
open AST.Raw;
open Util.RawUtil;

module Expression = Grammar.Expression;

module Assert =
  Assert.Make({
    type t = expression_t;

    let parser = ((_, ctx)) => ctx |> Expression.parser |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(ppf => Dump.expr_to_entity % Dump.Entity.pp(ppf), (==)),
          "program matches",
        )
      );
  });

let _generate_spaced_identifier_ops =
  List.map(((tag, op)) =>
    (
      (
        (
          "a" |> of_public |> as_raw_node |> of_id |> as_raw_node,
          "b" |> of_public |> as_raw_node |> of_id |> as_raw_node,
        )
        |> tag
        |> as_raw_node,
        "c" |> of_public |> as_raw_node |> of_id |> as_raw_node,
      )
      |> tag
      |> as_raw_node,
      Fmt.str("a %s b %s c", op, op),
    )
  );
let _generate_spaced_bool_ops = (~lhs=true, ~rhs=false) =>
  List.map(((tag, op)) =>
    [
      (
        (bool_prim(lhs), bool_prim(rhs)) |> tag |> as_raw_node,
        Fmt.str("%b%s%b", lhs, op, rhs),
      ),
      (
        (bool_prim(lhs), bool_prim(rhs)) |> tag |> as_raw_node,
        Fmt.str(" %b %s %b ", lhs, op, rhs),
      ),
    ]
  );
let _generate_spaced_int_ops = (~lhs=123, ~rhs=456) =>
  List.map(((tag, op)) =>
    [
      (
        (int_prim(lhs), int_prim(rhs)) |> tag |> as_raw_node,
        Fmt.str("%d%s%d", lhs, op, rhs),
      ),
      (
        (int_prim(lhs), int_prim(rhs)) |> tag |> as_raw_node,
        Fmt.str(" %d %s %d ", lhs, op, rhs),
      ),
    ]
  );

let _assert_parse_many = (~report=throw) =>
  List.iter(Tuple.join2(Assert.parse(~report)));

let suite =
  "Grammar.Expression"
  >::: [
    "no parse" >: (() => Assert.no_parse("~gibberish")),
    "parse primitive" >: (() => Assert.parse(123 |> int_prim, "123")),
    "parse public identifier"
    >: (
      () =>
        Assert.parse(
          ~report=ignore,
          "foo" |> of_public |> as_raw_node |> of_id |> as_raw_node,
          "foo",
        )
    ),
    "parse private identifier"
    >: (
      () =>
        Assert.parse(
          ~report=ignore,
          "foo" |> of_private |> as_raw_node |> of_id |> as_raw_node,
          "_foo",
        )
    ),
    "parse group"
    >: (
      () =>
        Assert.parse(
          "foo"
          |> of_public
          |> as_raw_node
          |> of_id
          |> as_raw_node
          |> of_group
          |> as_raw_node,
          "(foo)",
        )
    ),
    "parse closure"
    >: (
      () =>
        Assert.parse(
          [
            "foo"
            |> of_public
            |> as_raw_node
            |> of_id
            |> as_raw_node
            |> of_expr
            |> as_raw_node,
            ("x" |> of_public |> as_raw_node, false |> bool_prim)
            |> of_var
            |> as_raw_node,
            (
              "y" |> of_public |> as_raw_node,
              "foo" |> of_public |> as_raw_node |> of_id |> as_raw_node,
            )
            |> of_var
            |> as_raw_node,
            (
              "z" |> of_public |> as_raw_node,
              "y" |> of_public |> as_raw_node |> of_id |> as_raw_node,
            )
            |> of_var
            |> as_raw_node,
            (1 |> int_prim, 2 |> int_prim)
            |> of_add_op
            |> as_raw_node
            |> of_expr
            |> as_raw_node,
          ]
          |> of_closure
          |> as_raw_node,
          "{
            foo;
            let x = false;
            let y = foo;
            let z = y;
            1 + 2;
          }",
        )
    ),
    "parse unary - negative"
    >: (
      () => Assert.parse(123 |> int_prim |> of_neg_op |> as_raw_node, "-123")
    ),
    "parse unary - logical not"
    >: (
      () =>
        Assert.parse(true |> bool_prim |> of_not_op |> as_raw_node, "!true")
    ),
    "parse boolean logic"
    >: (
      () =>
        _assert_parse_many(
          [(of_and_op, "&&"), (of_or_op, "||")]
          |> _generate_spaced_bool_ops
          |> List.flatten,
        )
    ),
    "parse arithmetic"
    >: (
      () =>
        _assert_parse_many(
          [
            (of_add_op, "+"),
            (of_sub_op, "-"),
            (of_mult_op, "*"),
            (of_div_op, "/"),
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
            (of_lte_op, "<="),
            (of_lt_op, "<"),
            (of_gte_op, ">="),
            (of_gt_op, ">"),
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
                int_prim(2),
                (
                  int_prim(3),
                  (int_prim(4), int_prim(5)) |> of_expo_op |> as_raw_node,
                )
                |> of_mult_op
                |> as_raw_node,
              )
              |> of_add_op
              |> as_raw_node,
              (int_prim(6) |> of_neg_op |> as_raw_node, int_prim(7))
              |> of_div_op
              |> as_raw_node,
            )
            |> of_sub_op
            |> as_raw_node,
            "2 + 3 * 4 ^ 5 - -6 / 7",
          ),
          (
            (
              (int_prim(2), int_prim(3))
              |> of_add_op
              |> as_raw_node
              |> of_group
              |> as_raw_node,
              (
                int_prim(4),
                (
                  int_prim(5),
                  (int_prim(6), int_prim(7))
                  |> of_div_op
                  |> as_raw_node
                  |> of_group
                  |> as_raw_node
                  |> of_neg_op
                  |> as_raw_node,
                )
                |> of_sub_op
                |> as_raw_node
                |> of_group
                |> as_raw_node,
              )
              |> of_expo_op
              |> as_raw_node,
            )
            |> of_mult_op
            |> as_raw_node,
            "(2 + 3) * 4 ^ (5 - -(6 / 7))",
          ),
          (
            (
              (
                "a" |> of_public |> as_raw_node |> of_id |> as_raw_node,
                (
                  (
                    "b" |> of_public |> as_raw_node |> of_id |> as_raw_node,
                    "c" |> of_public |> as_raw_node |> of_id |> as_raw_node,
                  )
                  |> of_gt_op
                  |> as_raw_node,
                  (
                    "e" |> of_public |> as_raw_node |> of_id |> as_raw_node,
                    "f" |> of_public |> as_raw_node |> of_id |> as_raw_node,
                  )
                  |> of_lte_op
                  |> as_raw_node,
                )
                |> of_or_op
                |> as_raw_node
                |> of_group
                |> as_raw_node,
              )
              |> of_and_op
              |> as_raw_node,
              (
                "g" |> of_public |> as_raw_node |> of_id |> as_raw_node,
                "h" |> of_public |> as_raw_node |> of_id |> as_raw_node,
              )
              |> of_or_op
              |> as_raw_node
              |> of_group
              |> as_raw_node
              |> of_not_op
              |> as_raw_node
              |> of_group
              |> as_raw_node,
            )
            |> of_and_op
            |> as_raw_node,
            "a && (b > c || e <= f) && (!(g || h))",
          ),
        ])
    ),
    "parse left-associative - add, subtract, multiply"
    >: (
      () =>
        _assert_parse_many(
          [(of_add_op, "+"), (of_sub_op, "-"), (of_mult_op, "*")]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - divide"
    >: (
      () =>
        _assert_parse_many(
          [(of_div_op, "/")] |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - logical and, logical or"
    >: (
      () =>
        _assert_parse_many(
          [
            (of_lte_op, "<="),
            (of_lt_op, "<"),
            (of_gte_op, ">="),
            (of_gt_op, ">"),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - comparison"
    >: (
      () =>
        _assert_parse_many(
          ~report=ignore,
          [
            (of_lte_op, "<="),
            (of_lt_op, "<"),
            (of_gte_op, ">="),
            (of_gt_op, ">"),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - equality"
    >: (
      () =>
        _assert_parse_many(
          ~report=ignore,
          [(of_eq_op, "=="), (of_ineq_op, "!=")]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse right-associative - exponent"
    >: (
      () =>
        Assert.parse(
          (
            "a" |> of_public |> as_raw_node |> of_id |> as_raw_node,
            (
              "b" |> of_public |> as_raw_node |> of_id |> as_raw_node,
              "c" |> of_public |> as_raw_node |> of_id |> as_raw_node,
            )
            |> of_expo_op
            |> as_raw_node,
          )
          |> of_expo_op
          |> as_raw_node,
          "a ^ b ^ c",
        )
    ),
    "parse right-associative - negative"
    >: (
      () =>
        Assert.parse(
          "a"
          |> of_public
          |> as_raw_node
          |> of_id
          |> as_raw_node
          |> of_neg_op
          |> as_raw_node
          |> of_neg_op
          |> as_raw_node
          |> of_neg_op
          |> as_raw_node,
          "- - - a",
        )
    ),
    "parse right-associative - logical not"
    >: (
      () =>
        Assert.parse(
          "a"
          |> of_public
          |> as_raw_node
          |> of_id
          |> as_raw_node
          |> of_not_op
          |> as_raw_node
          |> of_not_op
          |> as_raw_node
          |> of_not_op
          |> as_raw_node,
          "! ! ! a",
        )
    ),
  ];
