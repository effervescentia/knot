open Kore;
open Parse.Kore;
open AST;

/*
  each expression has a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* || */
let rec expr_0 = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
  chainl1(expr_1(ctx), KLogicalOr.Plugin.parse)

/* && */
and expr_1 = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
  chainl1(expr_2(ctx), KLogicalAnd.Plugin.parse)

/* ==, != */
and expr_2 = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
  chainl1(expr_3(ctx), KEqual.Plugin.parse <|> KUnequal.Plugin.parse)

/* <=, <, >=, > */
and expr_3 = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
  chainl1(
    expr_4(ctx),
    choice([
      KLessOrEqual.Plugin.parse,
      KLessThan.Plugin.parse,
      KGreaterOrEqual.Plugin.parse,
      KGreaterThan.Plugin.parse,
    ]),
  )

/* +, - */
and expr_4 = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
  chainl1(expr_5(ctx), KAdd.Plugin.parse <|> KSubtract.Plugin.parse)

/* *, / */
and expr_5 = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
  chainl1(expr_6(ctx), KMultiply.Plugin.parse <|> KDivide.Plugin.parse)

/* ^ */
and expr_6 = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
  chainr1(expr_7(ctx), KExponentiate.Plugin.parse)

/* !, +, - */
and expr_7 = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
  expr_8(ctx) |> KUnaryOperator.parse

/* foo(bar) */
and expr_8 = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
  /* do not attempt to simplify this `input` argument away or expression parsing will loop forever */
  input => ((expr_9(ctx), expr_0(ctx)) |> KFunctionCall.parse)(input)

/* foo.bar */
and expr_9 = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
  expr_10(ctx) >>= KDotAccess.parse

/* {}, () */
and expr_10 = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
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

and jsx_term = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
  /* skip dot access to avoid conflict with class attribute syntax */
  (expr_10(ctx), expr_0(ctx)) |> KFunctionCall.parse |> KUnaryOperator.parse

/* 2, foo, <bar /> */
and term = (ctx: ParseContext.t): ParserTypes.expression_parser_t =>
  choice([
    () |> KPrimitive.parse,
    (ctx, expr_0) |> KStyle.parse,
    ctx |> KIdentifier.parse,
    (ctx, (jsx_term, expr_0)) |> KSX.parse,
  ]);

let expression = expr_0;
