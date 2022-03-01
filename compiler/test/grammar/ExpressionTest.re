open Kore;

module Expression = Grammar.Expression;
module U = Util.RawUtilV2;

module Assert =
  Assert.Make({
    type t = AR.expression_t;

    let parser = ((_, ctx)) =>
      ctx |> Expression.parser |> Assert.parse_completely |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(ppf => AR.Dump.(expr_to_entity % Entity.pp(ppf)), (==)),
          "program matches",
        )
      );
  });

let _generate_spaced_identifier_ops =
  List.map(((tag, op)) =>
    (
      (
        (
          "a" |> AR.of_public |> AR.of_id |> U.as_unknown,
          "b" |> AR.of_public |> AR.of_id |> U.as_unknown,
        )
        |> tag,
        "c" |> AR.of_public |> AR.of_id |> U.as_unknown,
      )
      |> tag,
      Fmt.str("a %s b %s c", op, op),
    )
  );
let _generate_spaced_bool_ops = (~lhs=true, ~rhs=false) =>
  List.map(((tag, op)) =>
    [
      (
        (U.bool_prim(lhs), U.bool_prim(rhs)) |> tag |> U.as_bool,
        Fmt.str("%b%s%b", lhs, op, rhs),
      ),
      (
        (U.bool_prim(lhs), U.bool_prim(rhs)) |> tag |> U.as_bool,
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

let _assert_parse_many = (~report=throw) =>
  List.iter(Tuple.join2(Assert.parse(~report)));

let suite =
  "Grammar.Expression"
  >::: [
    "no parse" >: (() => Assert.no_parse("~gibberish")),
    "parse primitive" >: (() => Assert.parse(U.int_prim(123), "123")),
    "parse identifier"
    >: (
      () =>
        Assert.parse("foo" |> AR.of_public |> AR.of_id |> U.as_unknown, "foo")
    ),
    "parse group"
    >: (
      () =>
        Assert.parse(
          "foo"
          |> AR.of_public
          |> AR.of_id
          |> U.as_unknown
          |> AR.of_group
          |> U.as_unknown,
          "(foo)",
        )
    ),
    "parse closure"
    >: (
      () =>
        Assert.parse(
          [
            "foo"
            |> AR.of_public
            |> AR.of_id
            |> U.as_unknown
            |> AR.of_expr
            |> U.as_unknown,
            ("x" |> AR.of_public |> U.as_raw_node, U.bool_prim(false))
            |> AR.of_var
            |> U.as_nil,
            (
              "y" |> AR.of_public |> U.as_raw_node,
              "foo" |> AR.of_public |> AR.of_id |> U.as_unknown,
            )
            |> AR.of_var
            |> U.as_nil,
            (
              "z" |> AR.of_public |> U.as_raw_node,
              "y" |> AR.of_public |> AR.of_id |> U.as_unknown,
            )
            |> AR.of_var
            |> U.as_nil,
            (U.int_prim(1), U.int_prim(2))
            |> AR.of_add_op
            |> U.as_int
            |> AR.of_expr
            |> U.as_int,
          ]
          |> AR.of_closure
          |> U.as_int,
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
      () =>
        Assert.parse(123 |> U.int_prim |> AR.of_neg_op |> U.as_int, "-123")
    ),
    "parse unary - logical not"
    >: (
      () =>
        Assert.parse(
          true |> U.bool_prim |> AR.of_not_op |> U.as_bool,
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
            (AR.of_add_op % U.as_int, "+"),
            (AR.of_sub_op % U.as_int, "-"),
            (AR.of_mult_op % U.as_int, "*"),
            (AR.of_div_op % U.as_float, "/"),
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
            (AR.of_lte_op % U.as_bool, "<="),
            (AR.of_lt_op % U.as_bool, "<"),
            (AR.of_gte_op % U.as_bool, ">="),
            (AR.of_gt_op % U.as_bool, ">"),
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
                  |> U.as_float,
                )
                |> AR.of_mult_op
                |> U.as_float,
              )
              |> AR.of_add_op
              |> U.as_float,
              (6 |> U.int_prim |> AR.of_neg_op |> U.as_int, U.int_prim(7))
              |> AR.of_div_op
              |> U.as_float,
            )
            |> AR.of_sub_op
            |> U.as_float,
            "2 + 3 * 4 ^ 5 - -6 / 7",
          ),
          (
            (
              (U.int_prim(2), U.int_prim(3))
              |> AR.of_add_op
              |> U.as_int
              |> AR.of_group
              |> U.as_int,
              (
                U.int_prim(4),
                (
                  U.int_prim(5),
                  (U.int_prim(6), U.int_prim(7))
                  |> AR.of_div_op
                  |> U.as_float
                  |> AR.of_group
                  |> U.as_float
                  |> AR.of_neg_op
                  |> U.as_float,
                )
                |> AR.of_sub_op
                |> U.as_float
                |> AR.of_group
                |> U.as_float,
              )
              |> AR.of_expo_op
              |> U.as_float,
            )
            |> AR.of_mult_op
            |> U.as_float,
            "(2 + 3) * 4 ^ (5 - -(6 / 7))",
          ),
          (
            (
              (
                "a" |> AR.of_public |> AR.of_id |> U.as_unknown,
                (
                  (
                    "b" |> AR.of_public |> AR.of_id |> U.as_unknown,
                    "c" |> AR.of_public |> AR.of_id |> U.as_unknown,
                  )
                  |> AR.of_gt_op
                  |> U.as_bool,
                  (
                    "e" |> AR.of_public |> AR.of_id |> U.as_unknown,
                    "f" |> AR.of_public |> AR.of_id |> U.as_unknown,
                  )
                  |> AR.of_lte_op
                  |> U.as_bool,
                )
                |> AR.of_or_op
                |> U.as_bool
                |> AR.of_group
                |> U.as_bool,
              )
              |> AR.of_and_op
              |> U.as_bool,
              (
                "g" |> AR.of_public |> AR.of_id |> U.as_unknown,
                "h" |> AR.of_public |> AR.of_id |> U.as_unknown,
              )
              |> AR.of_or_op
              |> U.as_bool
              |> AR.of_group
              |> U.as_bool
              |> AR.of_not_op
              |> U.as_bool
              |> AR.of_group
              |> U.as_bool,
            )
            |> AR.of_and_op
            |> U.as_bool,
            "a && (b > c || e <= f) && (!(g || h))",
          ),
        ])
    ),
    "parse left-associative - add, subtract, multiply"
    >: (
      () =>
        _assert_parse_many(
          [
            (AR.of_add_op % U.as_unknown, "+"),
            (AR.of_sub_op % U.as_unknown, "-"),
            (AR.of_mult_op % U.as_unknown, "*"),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - divide"
    >: (
      () =>
        _assert_parse_many(
          [(AR.of_div_op % U.as_float, "/")]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - logical and, logical or"
    >: (
      () =>
        _assert_parse_many(
          [
            (AR.of_and_op % U.as_bool, "&&"),
            (AR.of_or_op % U.as_bool, "||"),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - comparison"
    >: (
      () =>
        _assert_parse_many(
          [
            (AR.of_lte_op % U.as_bool, "<="),
            (AR.of_lt_op % U.as_bool, "<"),
            (AR.of_gte_op % U.as_bool, ">="),
            (AR.of_gt_op % U.as_bool, ">"),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse left-associative - equality"
    >: (
      () =>
        _assert_parse_many(
          [
            (AR.of_eq_op % U.as_bool, "=="),
            (AR.of_ineq_op % U.as_bool, "!="),
          ]
          |> _generate_spaced_identifier_ops,
        )
    ),
    "parse right-associative - exponent"
    >: (
      () =>
        Assert.parse(
          (
            "a" |> AR.of_public |> AR.of_id |> U.as_unknown,
            (
              "b" |> AR.of_public |> AR.of_id |> U.as_unknown,
              "c" |> AR.of_public |> AR.of_id |> U.as_unknown,
            )
            |> AR.of_expo_op
            |> U.as_float,
          )
          |> AR.of_expo_op
          |> U.as_float,
          "a ^ b ^ c",
        )
    ),
    "parse right-associative - negative"
    >: (
      () =>
        Assert.parse(
          "a"
          |> AR.of_public
          |> AR.of_id
          |> U.as_unknown
          |> AR.of_neg_op
          |> U.as_unknown
          |> AR.of_neg_op
          |> U.as_unknown
          |> AR.of_neg_op
          |> U.as_unknown,
          "- - - a",
        )
    ),
    "parse right-associative - logical not"
    >: (
      () =>
        Assert.parse(
          "a"
          |> AR.of_public
          |> AR.of_id
          |> U.as_unknown
          |> AR.of_not_op
          |> U.as_bool
          |> AR.of_not_op
          |> U.as_bool
          |> AR.of_not_op
          |> U.as_bool,
          "! ! ! a",
        )
    ),
  ];
