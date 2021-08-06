open Kore;
open AST.Raw;
open Type.Raw;

let primitive =
  Primitive.parser >|= (prim => (of_prim(prim), Node.Raw.get_range(prim)));

let identifier = (ctx: ModuleContext.t) =>
  Identifier.parser(ctx) >|= (id => (of_id(id), Node.Raw.get_range(id)));

let jsx = (ctx: ModuleContext.t, x) =>
  JSX.parser(ctx, x) >|= (jsx => (of_jsx(jsx), Node.Raw.get_range(jsx)));

let group = x =>
  M.between(Symbol.open_group, Symbol.close_group, x)
  >|= (
    node => {
      Node.Raw.(node |> get_value |> of_group, get_range(node));
    }
  );

let closure = (ctx: ModuleContext.t, x) =>
  Statement.parser(ctx, x)
  |> many
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  >|= Node.Raw.(Tuple.split2(get_value, get_range))
  >|= (
    ((stmts, range)) => {
      (of_closure(stmts), range);
    }
  );

/*
 each expression has a precedence denoted by its suffix

 the parser with the highest precedence should be matched first
 */

/* || */
let rec expr_0 = (ctx: ModuleContext.t, input) =>
  chainl1(expr_1(ctx), Operator.logical_or(ctx), input)

/* && */
and expr_1 = (ctx: ModuleContext.t, input) =>
  chainl1(expr_2(ctx), Operator.logical_and(ctx), input)

/* ==, != */
and expr_2 = (ctx: ModuleContext.t, input) =>
  chainl1(
    expr_3(ctx),
    Operator.equality(ctx) <|> Operator.inequality(ctx),
    input,
  )

/* <=, <, >=, > */
and expr_3 = (ctx: ModuleContext.t, input) =>
  chainl1(
    expr_4(ctx),
    choice([
      Operator.less_or_eql(ctx),
      Operator.less_than(ctx),
      Operator.greater_or_eql(ctx),
      Operator.greater_than(ctx),
    ]),
    input,
  )

/* +, - */
and expr_4 = (ctx: ModuleContext.t, input) =>
  chainl1(expr_5(ctx), Operator.add(ctx) <|> Operator.sub(ctx), input)

/* *, / */
and expr_5 = (ctx: ModuleContext.t, input) =>
  chainl1(expr_6(ctx), Operator.mult(ctx) <|> Operator.div(ctx), input)

/* ^ */
and expr_6 = (ctx: ModuleContext.t, input) =>
  chainr1(expr_7(ctx), Operator.expo(ctx), input)

/* !, +, - */
and expr_7 = (ctx: ModuleContext.t, input) =>
  M.unary_op(
    expr_8(ctx),
    choice([
      Operator.not(ctx),
      Operator.positive(ctx),
      Operator.negative(ctx),
    ]),
    input,
  )

/* {}, () */
and expr_8 = (ctx: ModuleContext.t, input) =>
  choice([closure(ctx, expr_0), expr_0(ctx) |> group, term(ctx)], input)

/* 2, foo, <bar /> */
and term = (ctx: ModuleContext.t, input) =>
  choice([primitive, identifier(ctx), jsx(ctx, (expr_4, expr_0))], input);

let parser = expr_0;
