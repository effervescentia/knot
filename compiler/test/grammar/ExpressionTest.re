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
          testable(pp => Dump.expr_to_entity % Dump.Entity.pp(pp), (==)),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Expression"
  >::: [
    "no parse" >: (() => ["~gibberish"] |> Assert.no_parse),
    "parse primitive" >: (() => Assert.parse("123", 123 |> int_prim)),
    "parse identifier"
    >: (
      () =>
        [
          ("foo", "foo" |> of_public |> as_raw_node |> of_id |> as_raw_node),
          ("bar", "bar" |> of_public |> as_raw_node |> of_id |> as_raw_node),
        ]
        |> Assert.parse_many(~report=ignore)
    ),
    "parse group"
    >: (
      () =>
        Assert.parse(
          "(foo)",
          "foo"
          |> of_public
          |> as_raw_node
          |> of_id
          |> as_raw_node
          |> of_group
          |> as_raw_node,
        )
    ),
    "parse closure"
    >: (
      () =>
        [
          (
            "{
            foo;
            let x = false;
            let y = foo;
            let z = y;
            1 + 2;
          }",
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
          ),
        ]
        |> Assert.parse_many
    ),
    "parse unary"
    >: (
      () =>
        [
          ("-123", 123 |> int_prim |> of_neg_op |> as_raw_node),
          ("!true", true |> bool_prim |> of_not_op |> as_raw_node),
        ]
        |> Assert.parse_many
    ),
    "parse boolean logic"
    >: (
      () =>
        [("&&", of_and_op), ("||", of_or_op)]
        |> List.map(((op, tag)) =>
             [
               (
                 op |> Fmt.str("true%sfalse"),
                 (true |> bool_prim, false |> bool_prim) |> tag |> as_raw_node,
               ),
               (
                 op |> Fmt.str(" true %s false "),
                 (true |> bool_prim, false |> bool_prim) |> tag |> as_raw_node,
               ),
             ]
           )
        |> List.flatten
        |> Assert.parse_many
    ),
    "parse arithmetic"
    >: (
      () =>
        [
          ("+", of_add_op % as_raw_node),
          ("-", of_sub_op % as_raw_node),
          ("*", of_mult_op % as_raw_node),
          ("/", of_div_op % as_raw_node),
        ]
        |> List.map(((op, tag)) =>
             [
               (
                 op |> Fmt.str("123%s456"),
                 (123 |> int_prim, 456 |> int_prim) |> tag,
               ),
               (
                 op |> Fmt.str(" 123 %s 456 "),
                 (123 |> int_prim, 456 |> int_prim) |> tag,
               ),
             ]
           )
        |> List.flatten
        |> Assert.parse_many
    ),
    "parse comparison"
    >: (
      () =>
        [
          ("<=", of_lte_op),
          ("<", of_lt_op),
          (">=", of_gte_op),
          (">", of_gt_op),
        ]
        |> List.map(((op, tag)) =>
             [
               (
                 op |> Fmt.str("123%s456"),
                 (123 |> int_prim, 456 |> int_prim) |> tag |> as_raw_node,
               ),
               (
                 op |> Fmt.str(" 123 %s 456 "),
                 (123 |> int_prim, 456 |> int_prim) |> tag |> as_raw_node,
               ),
             ]
           )
        |> List.flatten
        |> Assert.parse_many
    ),
    "parse complex expression"
    >: (
      () =>
        [
          (
            "2 + 3 * 4 ^ 5 - -6 / 7",
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
          ),
          (
            "(2 + 3) * 4 ^ (5 - -(6 / 7))",
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
          ),
          (
            "a && (b > c || e <= f) && (!(g || h))",
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
          ),
        ]
        |> Assert.parse_many
    ),
    "parse left-associative"
    >: (
      () => {
        [("+", of_add_op), ("-", of_sub_op), ("*", of_mult_op)]
        |> List.map(((op, tag)) =>
             (
               Fmt.str("a %s b %s c", op, op),
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
             )
           )
        |> Assert.parse_many;

        [("/", of_div_op)]
        |> List.map(((op, tag)) =>
             (
               Fmt.str("a %s b %s c", op, op),
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
             )
           )
        |> Assert.parse_many;

        [("&&", of_and_op), ("||", of_or_op)]
        |> List.map(((op, tag)) =>
             (
               Fmt.str("a %s b %s c", op, op),
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
             )
           )
        |> Assert.parse_many;

        [
          ("<=", of_lte_op),
          ("<", of_lt_op),
          (">=", of_gte_op),
          (">", of_gt_op),
        ]
        |> List.map(((op, tag)) =>
             (
               Fmt.str("a %s b %s c", op, op),
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
             )
           )
        |> Assert.parse_many(~report=ignore);

        [("==", of_eq_op), ("!=", of_ineq_op)]
        |> List.map(((op, tag)) =>
             (
               Fmt.str("a %s b %s c", op, op),
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
             )
           )
        |> Assert.parse_many(~report=ignore);
      }
    ),
    "parse right-associative"
    >: (
      () => {
        [
          (
            "a ^ b ^ c",
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
          ),
        ]
        |> Assert.parse_many;

        [
          (
            "- - - a",
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
          ),
        ]
        |> Assert.parse_many;

        [
          (
            "! ! ! a",
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
          ),
        ]
        |> Assert.parse_many;
      }
    ),
  ];
