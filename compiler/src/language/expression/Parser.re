open Kore;
open Parse.Kore;
open AST;

/*
  each expression has a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* &&, || */
let rec expr_0 = ctx => expr_1(ctx) |> KBinaryOperator.parse_logical

/* ==, != */
and expr_1 = ctx => expr_2(ctx) |> KBinaryOperator.parse_comparison

/* <=, <, >=, > */
and expr_2 = ctx => expr_3(ctx) |> KBinaryOperator.parse_relational

/* ^, *, /, +, - */
and expr_3 = ctx => expr_4(ctx) |> KBinaryOperator.parse_arithmetic

/* !, +, - */
and expr_4 = ctx => expr_5(ctx) |> KUnaryOperator.parse

/* foo(bar) */
and expr_5 = (ctx): Framework.expression_parser_t =>
  /* do not attempt to simplify this `input` argument away or expression parsing will loop forever */
  input => ((expr_6(ctx), expr_0(ctx)) |> KFunctionCall.parse)(input)

/* foo::bar */
and expr_6 = ctx => (ctx, expr_7) |> KBindStyle.parse

/* foo.bar */
and expr_7 = ctx => expr_8(ctx) >>= KDotAccess.parse

/* {}, () */
and expr_8 = (ctx): Framework.expression_parser_t =>
  /* do not attempt to simplify this `input` argument away or expression parsing will loop forever */
  input =>
    choice(
      [
        (ctx, expr_0) |> KClosure.parse,
        expr_0(ctx) |> KGroup.parse,
        term(ctx),
      ],
      input,
    )

/* skip dot access to avoid conflict with class attribute syntax */
and jsx_term = ctx =>
  (expr_8(ctx), expr_0(ctx)) |> KFunctionCall.parse |> KUnaryOperator.parse

/* 2, foo, <bar /> */
and term = ctx =>
  choice([
    () |> KPrimitive.parse,
    (ctx, expr_0) |> KStyle.parse,
    ctx |> KIdentifier.parse,
    (ctx, (jsx_term, expr_0)) |> KSX.parse,
  ]);

let expression = expr_0;
