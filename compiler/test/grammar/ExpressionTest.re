open Kore;
open AST.Raw;
open Util.RawUtil;

module Expression = Grammar.Expression;

module Assert =
  Assert.Make({
    type t = expression_t;

    let parser = ((_, _, ctx)) => ctx |> Expression.parser |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (pp, value) =>
              Debug.(print_lexeme("Expression", print_expr, value))
              |> Pretty.to_string
              |> Format.pp_print_string(pp),
            (==),
          ),
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
          ("foo", "foo" |> of_public |> as_lexeme |> of_id |> as_lexeme),
          ("bar", "bar" |> of_public |> as_lexeme |> of_id |> as_lexeme),
        ]
        |> Assert.parse_many(
             ~report=ignore,
             ~cls_context=scope_to_closure([("bar", Strong(`Integer))]),
           )
    ),
    "parse group"
    >: (
      () =>
        Assert.parse(
          ~cls_context=scope_to_closure([("foo", Strong(`Boolean))]),
          "(foo)",
          "foo"
          |> of_public
          |> as_lexeme
          |> of_id
          |> as_lexeme
          |> of_group
          |> as_lexeme,
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
              "foo" |> of_public |> as_lexeme |> of_id |> as_lexeme |> of_expr,
              ("x" |> of_public |> as_lexeme, false |> bool_prim) |> of_var,
              (
                "y" |> of_public |> as_lexeme,
                "foo" |> of_public |> as_lexeme |> of_id |> as_lexeme,
              )
              |> of_var,
              (
                "z" |> of_public |> as_lexeme,
                "y" |> of_public |> as_lexeme |> of_id |> as_lexeme,
              )
              |> of_var,
              (1 |> int_prim, 2 |> int_prim)
              |> of_add_op
              |> as_lexeme
              |> of_expr,
            ]
            |> of_closure
            |> as_lexeme,
          ),
        ]
        |> Assert.parse_many(
             ~cls_context=scope_to_closure([("foo", Strong(`String))]),
           )
    ),
    "parse unary"
    >: (
      () =>
        [
          ("-123", 123 |> int_prim |> of_neg_op |> as_lexeme),
          ("!true", true |> bool_prim |> of_not_op |> as_lexeme),
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
                 op |> Print.fmt("true%sfalse"),
                 (true |> bool_prim, false |> bool_prim) |> tag |> as_lexeme,
               ),
               (
                 op |> Print.fmt(" true %s false "),
                 (true |> bool_prim, false |> bool_prim) |> tag |> as_lexeme,
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
          ("+", of_add_op % as_lexeme),
          ("-", of_sub_op % as_lexeme),
          ("*", of_mult_op % as_lexeme),
          ("/", of_div_op % as_lexeme),
        ]
        |> List.map(((op, tag)) =>
             [
               (
                 op |> Print.fmt("123%s456"),
                 (123 |> int_prim, 456 |> int_prim) |> tag,
               ),
               (
                 op |> Print.fmt(" 123 %s 456 "),
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
                 op |> Print.fmt("123%s456"),
                 (123 |> int_prim, 456 |> int_prim) |> tag |> as_lexeme,
               ),
               (
                 op |> Print.fmt(" 123 %s 456 "),
                 (123 |> int_prim, 456 |> int_prim) |> tag |> as_lexeme,
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
                  (int_prim(4), int_prim(5)) |> of_expo_op |> as_lexeme,
                )
                |> of_mult_op
                |> as_lexeme,
              )
              |> of_add_op
              |> as_lexeme,
              (int_prim(6) |> of_neg_op |> as_lexeme, int_prim(7))
              |> of_div_op
              |> as_lexeme,
            )
            |> of_sub_op
            |> as_lexeme,
          ),
          (
            "(2 + 3) * 4 ^ (5 - -(6 / 7))",
            (
              (int_prim(2), int_prim(3))
              |> of_add_op
              |> as_lexeme
              |> of_group
              |> as_lexeme,
              (
                int_prim(4),
                (
                  int_prim(5),
                  (int_prim(6), int_prim(7))
                  |> of_div_op
                  |> as_lexeme
                  |> of_group
                  |> as_lexeme
                  |> of_neg_op
                  |> as_lexeme,
                )
                |> of_sub_op
                |> as_lexeme
                |> of_group
                |> as_lexeme,
              )
              |> of_expo_op
              |> as_lexeme,
            )
            |> of_mult_op
            |> as_lexeme,
          ),
          (
            "a && (b > c || e <= f) && (!(g || h))",
            (
              (
                "a" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                (
                  (
                    "b" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                    "c" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                  )
                  |> of_gt_op
                  |> as_lexeme,
                  (
                    "e" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                    "f" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                  )
                  |> of_lte_op
                  |> as_lexeme,
                )
                |> of_or_op
                |> as_lexeme
                |> of_group
                |> as_lexeme,
              )
              |> of_and_op
              |> as_lexeme,
              (
                "g" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                "h" |> of_public |> as_lexeme |> of_id |> as_lexeme,
              )
              |> of_or_op
              |> as_lexeme
              |> of_group
              |> as_lexeme
              |> of_not_op
              |> as_lexeme
              |> of_group
              |> as_lexeme,
            )
            |> of_and_op
            |> as_lexeme,
          ),
        ]
        |> Assert.parse_many(
             ~cls_context=
               scope_to_closure([
                 ("a", Strong(`Boolean)),
                 ("b", Strong(`Integer)),
                 ("c", Strong(`Float)),
                 ("e", Strong(`Float)),
                 ("f", Strong(`Integer)),
                 ("g", Strong(`Boolean)),
                 ("h", Strong(`Boolean)),
               ]),
           )
    ),
    "parse left-associative"
    >: (
      () => {
        [("+", of_add_op), ("-", of_sub_op), ("*", of_mult_op)]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                   "b" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                 )
                 |> tag
                 |> as_lexeme,
                 "c" |> of_public |> as_lexeme |> of_id |> as_lexeme,
               )
               |> tag
               |> as_lexeme,
             )
           )
        |> Assert.parse_many(
             ~cls_context=
               scope_to_closure([
                 ("a", Strong(`Integer)),
                 ("b", Strong(`Integer)),
                 ("c", Strong(`Integer)),
               ]),
           );

        [("/", of_div_op)]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                   "b" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                 )
                 |> tag
                 |> as_lexeme,
                 "c" |> of_public |> as_lexeme |> of_id |> as_lexeme,
               )
               |> tag
               |> as_lexeme,
             )
           )
        |> Assert.parse_many(
             ~cls_context=
               scope_to_closure([
                 ("a", Strong(`Integer)),
                 ("b", Strong(`Integer)),
                 ("c", Strong(`Integer)),
               ]),
           );

        [("&&", of_and_op), ("||", of_or_op)]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                   "b" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                 )
                 |> tag
                 |> as_lexeme,
                 "c" |> of_public |> as_lexeme |> of_id |> as_lexeme,
               )
               |> tag
               |> as_lexeme,
             )
           )
        |> Assert.parse_many(
             ~cls_context=
               scope_to_closure([
                 ("a", Strong(`Boolean)),
                 ("b", Strong(`Boolean)),
                 ("c", Strong(`Boolean)),
               ]),
           );
        [
          ("<=", of_lte_op),
          ("<", of_lt_op),
          (">=", of_gte_op),
          (">", of_gt_op),
        ]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                   "b" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                 )
                 |> tag
                 |> as_lexeme,
                 "c" |> of_public |> as_lexeme |> of_id |> as_lexeme,
               )
               |> tag
               |> as_lexeme,
             )
           )
        |> Assert.parse_many(
             ~report=ignore,
             ~cls_context=
               scope_to_closure([
                 ("a", weak_unknown),
                 ("b", weak_unknown),
                 ("c", weak_unknown),
               ]),
           );
        [("==", of_eq_op), ("!=", of_ineq_op)]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                   "b" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                 )
                 |> tag
                 |> as_lexeme,
                 "c" |> of_public |> as_lexeme |> of_id |> as_lexeme,
               )
               |> tag
               |> as_lexeme,
             )
           )
        |> Assert.parse_many(
             ~report=ignore,
             ~cls_context=
               scope_to_closure([
                 ("a", weak_unknown),
                 ("b", weak_unknown),
                 ("c", weak_unknown),
               ]),
           );
      }
    ),
    "parse right-associative"
    >: (
      () => {
        [
          (
            "a ^ b ^ c",
            (
              "a" |> of_public |> as_lexeme |> of_id |> as_lexeme,
              (
                "b" |> of_public |> as_lexeme |> of_id |> as_lexeme,
                "c" |> of_public |> as_lexeme |> of_id |> as_lexeme,
              )
              |> of_expo_op
              |> as_lexeme,
            )
            |> of_expo_op
            |> as_lexeme,
          ),
        ]
        |> Assert.parse_many(
             ~cls_context=
               scope_to_closure([
                 ("a", Strong(`Integer)),
                 ("b", Strong(`Integer)),
                 ("c", Strong(`Integer)),
               ]),
           );

        [
          (
            "- - - a",
            "a"
            |> of_public
            |> as_lexeme
            |> of_id
            |> as_lexeme
            |> of_neg_op
            |> as_lexeme
            |> of_neg_op
            |> as_lexeme
            |> of_neg_op
            |> as_lexeme,
          ),
        ]
        |> Assert.parse_many(
             ~cls_context=scope_to_closure([("a", Strong(`Integer))]),
           );
        [
          (
            "! ! ! a",
            "a"
            |> of_public
            |> as_lexeme
            |> of_id
            |> as_lexeme
            |> of_not_op
            |> as_lexeme
            |> of_not_op
            |> as_lexeme
            |> of_not_op
            |> as_lexeme,
          ),
        ]
        |> Assert.parse_many(
             ~cls_context=scope_to_closure([("a", Strong(`Boolean))]),
           );
      }
    ),
  ];
