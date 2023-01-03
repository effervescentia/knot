open Kore;
open Parse.Kore;

/*
  each expression has a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* &&, || */
let rec parse_expression_0 = ctx =>
  parse_expression_1(ctx) |> KBinaryOperator.parse_logical

/* ==, != */
and parse_expression_1 = ctx =>
  parse_expression_2(ctx) |> KBinaryOperator.parse_comparison

/* <=, <, >=, > */
and parse_expression_2 = ctx =>
  parse_expression_3(ctx) |> KBinaryOperator.parse_relational

/* ^, *, /, +, - */
and parse_expression_3 = ctx =>
  parse_expression_4(ctx) |> KBinaryOperator.parse_arithmetic

/* !, +, - */
and parse_expression_4 = ctx =>
  parse_expression_5(ctx) |> KUnaryOperator.parse

/* foo(bar) */
and parse_expression_5 = (ctx): AST.Framework.expression_parser_t =>
  /* do not attempt to simplify this `input` argument away or expression parsing will loop forever */
  input =>
    (
      (parse_expression_6(ctx), parse_expression_0(ctx))
      |> KFunctionCall.parse
    )(
      input,
    )

/* foo::bar */
and parse_expression_6 = ctx =>
  (ctx, (parse_expression_7, parse_expression_0)) |> KBindStyle.parse

/* foo.bar */
and parse_expression_7 = ctx => parse_expression_8(ctx) >>= KDotAccess.parse

/* {}, () */
and parse_expression_8 = (ctx): AST.Framework.expression_parser_t =>
  /* do not attempt to simplify this `input` argument away or expression parsing will loop forever */
  input =>
    choice(
      [
        (ctx, parse_expression_0) |> KClosure.parse,
        parse_expression_0(ctx) |> KGroup.parse,
        parse_term(ctx),
      ],
      input,
    )

/* skip dot access to avoid conflict with class attribute syntax */
and parse_ksx_term = ctx =>
  (parse_expression_8(ctx), parse_expression_0(ctx))
  |> KFunctionCall.parse
  |> KUnaryOperator.parse

/* 2, foo, <bar /> */
and parse_term = ctx =>
  choice([
    () |> KPrimitive.parse,
    (ctx, parse_expression_0) |> KStyle.parse,
    ctx |> KIdentifier.parse,
    (ctx, (parse_ksx_term, parse_expression_0)) |> KSX.parse,
  ]);

let parse = parse_expression_0;
