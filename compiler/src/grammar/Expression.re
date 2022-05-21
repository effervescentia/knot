open Kore;

let primitive: expression_parser_t =
  Primitive.parser >|= N.map_value(AR.of_prim);

let identifier = (ctx: ModuleContext.t): expression_parser_t =>
  Identifier.parser(ctx)
  >|= NR.map_value(AR.of_id)
  >|= N.of_raw(TR.(`Unknown));

let jsx =
    (ctx: ModuleContext.t, parsers: expression_parsers_arg_t)
    : expression_parser_t =>
  JSX.parser(ctx, parsers)
  >|= N.of_raw(TR.(`Element))
  >|= N.map_value(AR.of_jsx);

let group = (parse_expr: expression_parser_t): expression_parser_t =>
  M.between(Symbol.open_group, Symbol.close_group, parse_expr)
  >|= (
    ((expr, range)) =>
      N.create(AR.of_group(expr), N.get_type(expr), range)
  );

let closure =
    (ctx: ModuleContext.t, parse_expr: contextual_expression_parser_t)
    : expression_parser_t =>
  Statement.parser(ctx, parse_expr)
  |> many
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  >|= (
    ((stmts, range)) => {
      let last_stmt = List.last(stmts);

      N.create(
        AR.of_closure(stmts),
        /* if the statement list is empty the return type is nil */
        last_stmt |?> N.get_type |?: TR.(`Nil),
        range,
      );
    }
  );

/*
  each expression has a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* || */
let rec expr_0 = (ctx: ModuleContext.t): expression_parser_t =>
  chainl1(expr_1(ctx), Operator.logical_or(ctx))

/* && */
and expr_1 = (ctx: ModuleContext.t): expression_parser_t =>
  chainl1(expr_2(ctx), Operator.logical_and(ctx))

/* ==, != */
and expr_2 = (ctx: ModuleContext.t): expression_parser_t =>
  chainl1(expr_3(ctx), Operator.equality(ctx) <|> Operator.inequality(ctx))

/* <=, <, >=, > */
and expr_3 = (ctx: ModuleContext.t): expression_parser_t =>
  chainl1(
    expr_4(ctx),
    choice([
      Operator.less_or_eql(ctx),
      Operator.less_than(ctx),
      Operator.greater_or_eql(ctx),
      Operator.greater_than(ctx),
    ]),
  )

/* +, - */
and expr_4 = (ctx: ModuleContext.t): expression_parser_t =>
  chainl1(expr_5(ctx), Operator.add(ctx) <|> Operator.sub(ctx))

/* *, / */
and expr_5 = (ctx: ModuleContext.t): expression_parser_t =>
  chainl1(expr_6(ctx), Operator.mult(ctx) <|> Operator.div(ctx))

/* ^ */
and expr_6 = (ctx: ModuleContext.t): expression_parser_t =>
  chainr1(expr_7(ctx), Operator.expo(ctx))

/* !, +, - */
and expr_7 = (ctx: ModuleContext.t): expression_parser_t =>
  M.unary_op(
    expr_8(ctx),
    choice([
      Operator.not(ctx),
      Operator.positive(ctx),
      Operator.negative(ctx),
    ]),
  )

/* {}, () */
and expr_8 = (ctx: ModuleContext.t): expression_parser_t =>
  /* do not attempt to simplify this `input` argument away or expression parsing will loop forever */
  input =>
    choice([closure(ctx, expr_0), expr_0(ctx) |> group, term(ctx)], input)

/* 2, foo, <bar /> */
and term = (ctx: ModuleContext.t): expression_parser_t =>
  choice([primitive, identifier(ctx), jsx(ctx, (expr_4, expr_0))]);

let parser = expr_0;
