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
          testable(
            (x, y) => AST.print_expr(y) |> Format.print_string,
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Expression"
  >::: [
    "no parse" >: (() => ["~gibberish"] |> Assert.no_parse),
    "parse primitive" >: (() => Assert.parse("123", int_prim(123))),
    "parse boolean logic"
    >: (
      () =>
        [("&&", AST.of_and_op), ("||", AST.of_or_op)]
        |> List.map(((op, tag)) =>
             (
               op |> Print.fmt("true %s false"),
               (bool_prim(true), bool_prim(false)) |> tag,
             )
           )
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
             (
               op |> Print.fmt("123 %s 456"),
               (int_prim(123), int_prim(456)) |> tag,
             )
           )
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
             (
               op |> Print.fmt("123 %s 456"),
               (int_prim(123), int_prim(456)) |> tag,
             )
           )
        |> Assert.parse_many
    ),
    "parse complex arithmetic"
    >: (
      () =>
        Assert.parse(
          "2 + 3 * 4 - 5 / 6",
          (
            (int_prim(2), (int_prim(3), int_prim(4)) |> AST.of_mult_op)
            |> AST.of_add_op,
            (int_prim(5), int_prim(6)) |> AST.of_div_op,
          )
          |> AST.of_sub_op,
        )
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
               ((AST.of_id("a"), AST.of_id("b")) |> tag, AST.of_id("c"))
               |> tag,
             )
           )
        |> Assert.parse_many
    ),
  ];
