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
          testable(
            pp =>
              Tuple.fst3
              % Debug.print_expr
              % Pretty.to_string
              % Format.pp_print_string(pp),
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
        Assert.parse(
          "foo",
          "foo" |> of_public |> as_lexeme |> of_id |> as_unknown,
        )
    ),
    "parse group"
    >: (
      () =>
        Assert.parse(
          "(foo)",
          "foo"
          |> of_public
          |> as_lexeme
          |> of_id
          |> as_unknown
          |> of_group
          |> as_unknown,
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
            "foo" |> of_public |> as_lexeme |> of_id |> as_unknown |> of_expr,
            ("x" |> of_public |> as_lexeme, false |> bool_prim) |> of_var,
            (1 |> int_prim, 2 |> int_prim) |> of_add_op |> as_int |> of_expr,
          ]
          |> of_closure
          |> as_int,
        )
    ),
    "parse unary"
    >: (
      () =>
        [
          ("-123", 123 |> int_prim |> of_neg_op |> as_int),
          ("!true", true |> bool_prim |> of_not_op |> as_bool),
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
                 (true |> bool_prim, false |> bool_prim) |> tag |> as_bool,
               ),
               (
                 op |> Print.fmt(" true %s false "),
                 (true |> bool_prim, false |> bool_prim) |> tag |> as_bool,
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
          ("+", of_add_op % as_int),
          ("-", of_sub_op % as_int),
          ("*", of_mult_op % as_int),
          ("/", of_div_op % as_float),
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
                 (123 |> int_prim, 456 |> int_prim) |> tag |> as_bool,
               ),
               (
                 op |> Print.fmt(" 123 %s 456 "),
                 (123 |> int_prim, 456 |> int_prim) |> tag |> as_bool,
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
                  (int_prim(4), int_prim(5)) |> of_expo_op |> as_float,
                )
                |> of_mult_op
                |> as_float,
              )
              |> of_add_op
              |> as_float,
              (int_prim(6) |> of_neg_op |> as_int, int_prim(7))
              |> of_div_op
              |> as_float,
            )
            |> of_sub_op
            |> as_float,
          ),
          (
            "(2 + 3) * 4 ^ (5 - -(6 / 7))",
            (
              (int_prim(2), int_prim(3))
              |> of_add_op
              |> as_int
              |> of_group
              |> as_int,
              (
                int_prim(4),
                (
                  int_prim(5),
                  (int_prim(6), int_prim(7))
                  |> of_div_op
                  |> as_float
                  |> of_group
                  |> as_float
                  |> of_neg_op
                  |> as_float,
                )
                |> of_sub_op
                |> as_float
                |> of_group
                |> as_float,
              )
              |> of_expo_op
              |> as_float,
            )
            |> of_mult_op
            |> as_float,
          ),
          (
            "a && (b > c || e <= f) && (!(g || h))",
            (
              (
                "a" |> of_public |> as_lexeme |> of_id |> as_unknown,
                (
                  (
                    "b" |> of_public |> as_lexeme |> of_id |> as_unknown,
                    "c" |> of_public |> as_lexeme |> of_id |> as_unknown,
                  )
                  |> of_gt_op
                  |> as_bool,
                  (
                    "e" |> of_public |> as_lexeme |> of_id |> as_unknown,
                    "f" |> of_public |> as_lexeme |> of_id |> as_unknown,
                  )
                  |> of_lte_op
                  |> as_bool,
                )
                |> of_or_op
                |> as_bool
                |> of_group
                |> as_bool,
              )
              |> of_and_op
              |> as_bool,
              (
                "g" |> of_public |> as_lexeme |> of_id |> as_unknown,
                "h" |> of_public |> as_lexeme |> of_id |> as_unknown,
              )
              |> of_or_op
              |> as_bool
              |> of_group
              |> as_bool
              |> of_not_op
              |> as_bool
              |> of_group
              |> as_bool,
            )
            |> of_and_op
            |> as_bool,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse left-associative"
    >: (
      () =>
        [
          ("+", of_add_op % as_invalid),
          ("-", of_sub_op % as_invalid),
          ("*", of_mult_op % as_invalid),
          ("/", of_div_op % as_float),
          ("&&", of_and_op % as_bool),
          ("||", of_or_op % as_bool),
          ("<=", of_lte_op % as_bool),
          ("<", of_lt_op % as_bool),
          (">=", of_gte_op % as_bool),
          (">", of_gt_op % as_bool),
          ("==", of_eq_op % as_bool),
          ("!=", of_ineq_op % as_bool),
        ]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               (
                 (
                   "a" |> of_public |> as_lexeme |> of_id |> as_unknown,
                   "b" |> of_public |> as_lexeme |> of_id |> as_unknown,
                 )
                 |> tag,
                 "c" |> of_public |> as_lexeme |> of_id |> as_unknown,
               )
               |> tag,
             )
           )
        |> Assert.parse_many
    ),
    "parse right-associative"
    >: (
      () => {
        [
          (
            "a ^ b ^ c",
            (
              "a" |> of_public |> as_lexeme |> of_id |> as_unknown,
              (
                "b" |> of_public |> as_lexeme |> of_id |> as_unknown,
                "c" |> of_public |> as_lexeme |> of_id |> as_unknown,
              )
              |> of_expo_op
              |> as_float,
            )
            |> of_expo_op
            |> as_float,
          ),
        ]
        |> Assert.parse_many;

        [
          (
            "- - - a",
            "a"
            |> of_public
            |> as_lexeme
            |> of_id
            |> as_unknown
            |> of_neg_op
            |> as_unknown
            |> of_neg_op
            |> as_unknown
            |> of_neg_op
            |> as_unknown,
          ),
        ]
        |> Assert.parse_many;
        [
          (
            "! ! ! a",
            "a"
            |> of_public
            |> as_lexeme
            |> of_id
            |> as_unknown
            |> of_not_op
            |> as_unknown
            |> of_not_op
            |> as_unknown
            |> of_not_op
            |> as_unknown,
          ),
        ]
        |> Assert.parse_many;
      }
    ),
  ];
