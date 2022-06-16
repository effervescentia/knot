open Kore;

let primitive: expression_parser_t = Primitive.parser >|= N2.map(AR.of_prim);

let identifier = (ctx: ModuleContext.t): expression_parser_t =>
  Identifier.parser(ctx) >|= N2.map(AR.of_id) >|= N2.add_type(TR.(`Unknown));

let jsx =
    (ctx: ModuleContext.t, parsers: expression_parsers_arg_t)
    : expression_parser_t =>
  JSX.parser(ctx, parsers)
  >|= N2.add_type(TR.(`Element))
  >|= N2.map(AR.of_jsx);

let group = (parse_expr: expression_parser_t): expression_parser_t =>
  M.between(Symbol.open_group, Symbol.close_group, parse_expr)
  >|= (
    ((expr, _) as expr_node) =>
      N2.typed(
        AR.of_group(expr),
        N2.get_type(expr),
        N2.get_range(expr_node),
      )
  );

let closure =
    (ctx: ModuleContext.t, parse_expr: contextual_expression_parser_t)
    : expression_parser_t =>
  Statement.parser(ctx, parse_expr)
  |> many
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  >|= (
    ((stmts, _) as stmts_node) => {
      let last_stmt = List.last(stmts);

      N2.typed(
        AR.of_closure(stmts),
        /* if the statement list is empty the return type is nil */
        last_stmt |?> N2.get_type |?: TR.(`Nil),
        N2.get_range(stmts_node),
      );
    }
  );

let dot_access = {
  let rec loop = expr =>
    Symbol.period
    >> M.identifier
    >>= (
      prop =>
        loop(
          N2.typed(
            (expr, prop) |> AR.of_dot_access,
            (
              switch (N2.get_type(expr)) {
              | `Struct(props) => props |> List.assoc_opt(fst(prop))
              | _ => None
              }
            )
            |?: TR.(`Unknown),
            N2.get_range(prop),
          ),
        )
    )
    |> option(expr);

  loop;
};

let function_call =
    (parse_term: expression_parser_t, parse_expr: expression_parser_t) => {
  let rec loop = expr =>
    parse_expr
    |> M.comma_sep
    |> M.between(Symbol.open_group, Symbol.close_group)
    >>= (
      args =>
        loop(
          N2.typed(
            (expr, fst(args)) |> AR.of_func_call,
            switch (N2.get_type(expr)) {
            | `Function(_, result) => result
            | _ => TR.(`Unknown)
            },
            N2.get_range(args),
          ),
        )
    )
    |> option(expr);

  parse_term >>= loop;
};

let unary_op =
    (ctx: ModuleContext.t, parse_expr: expression_parser_t)
    : expression_parser_t =>
  M.unary_op(
    parse_expr,
    choice([
      Operator.not(ctx),
      Operator.positive(ctx),
      Operator.negative(ctx),
    ]),
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
  unary_op(ctx, expr_8(ctx))

/* foo(bar) */
and expr_8 = (ctx: ModuleContext.t): expression_parser_t =>
  /* do not attempt to simplify this `input` argument away or expression parsing will loop forever */
  input => (function_call(expr_9(ctx), expr_0(ctx)))(input)

/* foo.bar */
and expr_9 = (ctx: ModuleContext.t): expression_parser_t =>
  expr_10(ctx) >>= dot_access

/* {}, () */
and expr_10 = (ctx: ModuleContext.t): expression_parser_t =>
  /* do not attempt to simplify this `input` argument away or expression parsing will loop forever */
  input =>
    choice([closure(ctx, expr_0), expr_0(ctx) |> group, term(ctx)], input)

and jsx_term = (ctx: ModuleContext.t): expression_parser_t =>
  /* skip dot access to avoid conflict with class attribute syntax */
  unary_op(ctx, function_call(expr_10(ctx), expr_0(ctx)))

/* 2, foo, <bar /> */
and term = (ctx: ModuleContext.t): expression_parser_t =>
  choice([primitive, identifier(ctx), jsx(ctx, (jsx_term, expr_0))]);

let parser = expr_0;
