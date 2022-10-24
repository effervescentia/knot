open Knot.Kore;
open Parse.Onyx;

let primitive: Grammar.Kore.expression_parser_t =
  KPrimitive.Plugin.parse >|= Node.map(AST.Raw.of_prim);

let identifier = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  KIdentifier.Plugin.parse(ctx)
  >|= Node.map(AST.Raw.of_id)
  >|= Node.add_type(Type.Raw.(`Unknown));

/*
  each expression has a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* || */
let rec expr_0 = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  chainl1(expr_1(ctx), KLogicalOr.Plugin.parse(ctx))

/* && */
and expr_1 = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  chainl1(expr_2(ctx), KLogicalAnd.Plugin.parse(ctx))

/* ==, != */
and expr_2 = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  chainl1(
    expr_3(ctx),
    KEqual.Plugin.parse(ctx) <|> KUnequal.Plugin.parse(ctx),
  )

/* <=, <, >=, > */
and expr_3 = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  chainl1(
    expr_4(ctx),
    choice([
      KLessOrEqual.Plugin.parse(ctx),
      KLessThan.Plugin.parse(ctx),
      KGreaterOrEqual.Plugin.parse(ctx),
      KGreaterThan.Plugin.parse(ctx),
    ]),
  )

/* +, - */
and expr_4 = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  chainl1(
    expr_5(ctx),
    KAdd.Plugin.parse(ctx) <|> KSubtract.Plugin.parse(ctx),
  )

/* *, / */
and expr_5 = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  chainl1(
    expr_6(ctx),
    KMultiply.Plugin.parse(ctx) <|> KDivide.Plugin.parse(ctx),
  )

/* ^ */
and expr_6 = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  chainr1(expr_7(ctx), KExponentiate.Plugin.parse(ctx))

/* !, +, - */
and expr_7 = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  expr_8(ctx) |> KUnaryOperator.Plugin.parse(ctx)

/* foo(bar) */
and expr_8 = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  /* do not attempt to simplify this `input` argument away or expression parsing will loop forever */
  input => (KFunctionCall.Plugin.parse(expr_9(ctx), expr_0(ctx)))(input)

/* foo.bar */
and expr_9 = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  expr_10(ctx) >>= KDotAccess.Plugin.parse

/* {}, () */
and expr_10 = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  /* do not attempt to simplify this `input` argument away or expression parsing will loop forever */
  input =>
    choice(
      [
        KStatement.Plugin.parse(ctx, expr_0) |> KClosure.Plugin.parse,
        expr_0(ctx) |> KGroup.Plugin.parse,
        term(ctx),
      ],
      input,
    )

and jsx_term = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  /* skip dot access to avoid conflict with class attribute syntax */
  KFunctionCall.Plugin.parse(expr_10(ctx), expr_0(ctx))
  |> KUnaryOperator.Plugin.parse(ctx)

/* 2, foo, <bar /> */
and term = (ctx: ParseContext.t): Grammar.Kore.expression_parser_t =>
  choice([
    primitive,
    expr_0 |> KStyle.Plugin.parse(ctx),
    identifier(ctx),
    (jsx_term, expr_0) |> KSX.Plugin.parse(ctx),
  ]);

let expression = expr_0;