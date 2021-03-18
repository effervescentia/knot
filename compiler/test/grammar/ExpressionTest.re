open Kore;
open AST;
open Util;

module Expression = Grammar.Expression;

module Assert =
  Assert.Make({
    type t = expression_t;

    let parser = Parser.parse(Expression.parser);

    let test =
      Alcotest.(
        check(
          testable(pp => fmt_expr % Format.pp_print_string(pp), (==)),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Expression"
  >::: [
    "no parse" >: (() => ["~gibberish"] |> Assert.no_parse),
    "parse primitive" >: (() => Assert.parse("123", int_prim(123))),
    "parse identifier"
    >: (() => Assert.parse("foo", "foo" |> of_public |> as_lexeme |> of_id)),
    "parse group"
    >: (
      () =>
        Assert.parse(
          "(foo)",
          "foo"
          |> of_public
          |> as_lexeme
          |> of_id
          |> to_block(~type_=Type.K_Unknown)
          |> of_group,
        )
    ),
    "parse closure"
    >: (
      () =>
        Assert.parse(
          "{
            foo;
            let x = false;
            1 + 2;
          }",
          [
            "foo" |> of_public |> as_lexeme |> of_id |> of_expr,
            ("x" |> of_public |> as_lexeme, bool_prim(false)) |> of_var,
            (int_prim(1), int_prim(2)) |> of_add_op |> of_expr,
          ]
          |> to_block(~type_=Type.K_Integer)
          |> of_closure,
        )
    ),
    "parse unary"
    >: (
      () =>
        [
          ("-123", int_prim(123) |> of_neg_op),
          ("!true", bool_prim(true) |> of_not_op),
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
                 (bool_prim(true), bool_prim(false)) |> tag,
               ),
               (
                 op |> Print.fmt(" true %s false "),
                 (bool_prim(true), bool_prim(false)) |> tag,
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
          ("+", of_add_op),
          ("-", of_sub_op),
          ("*", of_mult_op),
          ("/", of_div_op),
        ]
        |> List.map(((op, tag)) =>
             [
               (
                 op |> Print.fmt("123%s456"),
                 (int_prim(123), int_prim(456)) |> tag,
               ),
               (
                 op |> Print.fmt(" 123 %s 456 "),
                 (int_prim(123), int_prim(456)) |> tag,
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
                 (int_prim(123), int_prim(456)) |> tag,
               ),
               (
                 op |> Print.fmt(" 123 %s 456 "),
                 (int_prim(123), int_prim(456)) |> tag,
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
                (int_prim(3), (int_prim(4), int_prim(5)) |> of_expo_op)
                |> of_mult_op,
              )
              |> of_add_op,
              (int_prim(6) |> of_neg_op, int_prim(7)) |> of_div_op,
            )
            |> of_sub_op,
          ),
          (
            "(2 + 3) * 4 ^ (5 - -(6 / 7))",
            (
              (int_prim(2), int_prim(3))
              |> of_add_op
              |> to_block(~type_=Type.K_Integer)
              |> of_group,
              (
                int_prim(4),
                (
                  int_prim(5),
                  (int_prim(6), int_prim(7))
                  |> of_div_op
                  |> to_block(~type_=Type.K_Float)
                  |> of_group
                  |> of_neg_op,
                )
                |> of_sub_op
                |> to_block(~type_=Type.K_Float)
                |> of_group,
              )
              |> of_expo_op,
            )
            |> of_mult_op,
          ),
          (
            "a && (b > c || e <= f) && (!(g || h))",
            (
              (
                "a" |> of_public |> as_lexeme |> of_id,
                (
                  (
                    "b" |> of_public |> as_lexeme |> of_id,
                    "c" |> of_public |> as_lexeme |> of_id,
                  )
                  |> of_gt_op,
                  (
                    "e" |> of_public |> as_lexeme |> of_id,
                    "f" |> of_public |> as_lexeme |> of_id,
                  )
                  |> of_lte_op,
                )
                |> of_or_op
                |> to_block(~type_=Type.K_Boolean)
                |> of_group,
              )
              |> of_and_op,
              (
                "g" |> of_public |> as_lexeme |> of_id,
                "h" |> of_public |> as_lexeme |> of_id,
              )
              |> of_or_op
              |> to_block(~type_=Type.K_Boolean)
              |> of_group
              |> of_not_op
              |> to_block(~type_=Type.K_Boolean)
              |> of_group,
            )
            |> of_and_op,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse left-associative"
    >: (
      () =>
        [
          ("+", of_add_op),
          ("-", of_sub_op),
          ("*", of_mult_op),
          ("/", of_div_op),
          ("&&", of_and_op),
          ("||", of_or_op),
          ("<=", of_lte_op),
          ("<", of_lt_op),
          (">=", of_gte_op),
          (">", of_gt_op),
          ("==", of_eq_op),
          ("!=", of_ineq_op),
        ]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a" |> of_public |> as_lexeme |> of_id,
                   "b" |> of_public |> as_lexeme |> of_id,
                 )
                 |> tag,
                 "c" |> of_public |> as_lexeme |> of_id,
               )
               |> tag,
             )
           )
        |> Assert.parse_many
    ),
    "parse right-associative"
    >: (
      () =>
        (
          [("^", of_expo_op)]
          |> List.map(((op, tag)) =>
               (
                 Print.fmt("a %s b %s c", op, op),
                 (
                   "a" |> of_public |> as_lexeme |> of_id,
                   (
                     "b" |> of_public |> as_lexeme |> of_id,
                     "c" |> of_public |> as_lexeme |> of_id,
                   )
                   |> tag,
                 )
                 |> tag,
               )
             )
        )
        @ (
          [("-", of_neg_op), ("!", of_not_op)]
          |> List.map(((op, tag)) =>
               (
                 Print.fmt("%s %s %s a", op, op, op),
                 "a" |> of_public |> as_lexeme |> of_id |> tag |> tag |> tag,
               )
             )
        )
        |> Assert.parse_many
    ),
  ];
