open Kore;
open Util;

module Expression = Grammar.Expression;

module Assert =
  Assert.Make({
    type t = AST.expression_t;

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
    "parse identifier" >: (() => Assert.parse("foo", inv_id("foo"))),
    "parse group"
    >: (
      () =>
        Assert.parse(
          "(foo)",
          inv_id("foo") |> to_block(~type_=Type.K_Unknown) |> AST.of_group,
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
            inv_id("foo") |> AST.of_expr,
            ("x", bool_prim(false)) |> AST.of_var,
            (int_prim(1), int_prim(2)) |> AST.of_add_op |> AST.of_expr,
          ]
          |> to_block(~type_=Type.K_Integer)
          |> AST.of_closure,
        )
    ),
    "parse unary"
    >: (
      () =>
        [
          ("-123", int_prim(123) |> AST.of_neg_op),
          ("!true", bool_prim(true) |> AST.of_not_op),
        ]
        |> Assert.parse_many
    ),
    "parse boolean logic"
    >: (
      () =>
        [("&&", AST.of_and_op), ("||", AST.of_or_op)]
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
          ("+", AST.of_add_op),
          ("-", AST.of_sub_op),
          ("*", AST.of_mult_op),
          ("/", AST.of_div_op),
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
          ("<=", AST.of_lte_op),
          ("<", AST.of_lt_op),
          (">=", AST.of_gte_op),
          (">", AST.of_gt_op),
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
                (
                  int_prim(3),
                  (int_prim(4), int_prim(5)) |> AST.of_expo_op,
                )
                |> AST.of_mult_op,
              )
              |> AST.of_add_op,
              (int_prim(6) |> AST.of_neg_op, int_prim(7)) |> AST.of_div_op,
            )
            |> AST.of_sub_op,
          ),
          (
            "(2 + 3) * 4 ^ (5 - -(6 / 7))",
            (
              (int_prim(2), int_prim(3))
              |> AST.of_add_op
              |> to_block(~type_=Type.K_Integer)
              |> AST.of_group,
              (
                int_prim(4),
                (
                  int_prim(5),
                  (int_prim(6), int_prim(7))
                  |> AST.of_div_op
                  |> to_block(~type_=Type.K_Float)
                  |> AST.of_group
                  |> AST.of_neg_op,
                )
                |> AST.of_sub_op
                |> to_block(~type_=Type.K_Float)
                |> AST.of_group,
              )
              |> AST.of_expo_op,
            )
            |> AST.of_mult_op,
          ),
          (
            "a && (b > c || e <= f) && (!(g || h))",
            (
              (
                inv_id("a"),
                (
                  (inv_id("b"), inv_id("c")) |> AST.of_gt_op,
                  (inv_id("e"), inv_id("f")) |> AST.of_lte_op,
                )
                |> AST.of_or_op
                |> to_block(~type_=Type.K_Boolean)
                |> AST.of_group,
              )
              |> AST.of_and_op,
              (inv_id("g"), inv_id("h"))
              |> AST.of_or_op
              |> to_block(~type_=Type.K_Boolean)
              |> AST.of_group
              |> AST.of_not_op
              |> to_block(~type_=Type.K_Boolean)
              |> AST.of_group,
            )
            |> AST.of_and_op,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse left-associative"
    >: (
      () =>
        [
          ("+", AST.of_add_op),
          ("-", AST.of_sub_op),
          ("*", AST.of_mult_op),
          ("/", AST.of_div_op),
          ("&&", AST.of_and_op),
          ("||", AST.of_or_op),
          ("<=", AST.of_lte_op),
          ("<", AST.of_lt_op),
          (">=", AST.of_gte_op),
          (">", AST.of_gt_op),
          ("==", AST.of_eq_op),
          ("!=", AST.of_ineq_op),
        ]
        |> List.map(((op, tag)) =>
             (
               Print.fmt("a %s b %s c", op, op),
               ((inv_id("a"), inv_id("b")) |> tag, inv_id("c")) |> tag,
             )
           )
        |> Assert.parse_many
    ),
    "parse right-associative"
    >: (
      () =>
        (
          [("^", AST.of_expo_op)]
          |> List.map(((op, tag)) =>
               (
                 Print.fmt("a %s b %s c", op, op),
                 (inv_id("a"), (inv_id("b"), inv_id("c")) |> tag) |> tag,
               )
             )
        )
        @ (
          [("-", AST.of_neg_op), ("!", AST.of_not_op)]
          |> List.map(((op, tag)) =>
               (
                 Print.fmt("%s %s %s a", op, op, op),
                 inv_id("a") |> tag |> tag |> tag,
               )
             )
        )
        |> Assert.parse_many
    ),
  ];
