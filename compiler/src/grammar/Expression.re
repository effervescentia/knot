open Kore;

let _wrap_typed_lexeme = (f, (_, type_, cursor) as lexeme) => (
  f(lexeme),
  type_,
  cursor,
);

let primitive =
  Primitive.parser
  >|= (((_, type_, cursor) as prim) => (AST.of_prim(prim), type_, cursor));

let identifier = (ctx: ClosureContext.t) =>
  Identifier.parser(ctx)
  >|= (
    ((_, cursor) as id) => (
      AST.of_id(id),
      ctx |> ClosureContext.resolve(id),
      cursor,
    )
  );

let jsx = (ctx: ClosureContext.t, x) =>
  JSX.parser(ctx, x)
  >|= (
    ((_, cursor) as jsx) => (AST.of_jsx(jsx), `Strong(`Element), cursor)
  );

let group = x =>
  M.between(Symbol.open_group, Symbol.close_group, x)
  >|= (
    block => {
      let (_, type_, _) as group = block |> Block.value;

      (group |> AST.of_group, type_, block |> Block.cursor);
    }
  );

let closure = (ctx: ClosureContext.t, x) =>
  Statement.parser(ctx, x)
  |> many
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  >|= Tuple.split2(Block.value, Block.cursor)
  >|= (
    ((stmts, cursor)) => {
      let type_ =
        stmts
        |> List.last
        |> (
          fun
          | None => `Strong(`Nil)
          | Some(x) => TypeOf.statement(x)
        );

      (stmts |> AST.of_closure, type_, cursor);
    }
  );

/*
 each expression has a precedence denoted by its suffix

 the parser with the highest precedence should be matched first
 */

/* || */
let rec expr_0 = (ctx: ClosureContext.t, input) =>
  chainl1(expr_1(ctx), Operator.logical_or(ctx), input)

/* && */
and expr_1 = (ctx: ClosureContext.t, input) =>
  chainl1(expr_2(ctx), Operator.logical_and(ctx), input)

/* ==, != */
and expr_2 = (ctx: ClosureContext.t, input) =>
  chainl1(
    expr_3(ctx),
    Operator.equality(ctx) <|> Operator.inequality(ctx),
    input,
  )

/* <=, <, >=, > */
and expr_3 = (ctx: ClosureContext.t, input) =>
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
and expr_4 = (ctx: ClosureContext.t, input) =>
  chainl1(expr_5(ctx), Operator.add(ctx) <|> Operator.sub(ctx), input)

/* *, / */
and expr_5 = (ctx: ClosureContext.t, input) =>
  chainl1(expr_6(ctx), Operator.mult(ctx) <|> Operator.div(ctx), input)

/* ^ */
and expr_6 = (ctx: ClosureContext.t, input) =>
  chainr1(expr_7(ctx), Operator.expo(ctx), input)

/* !, +, - */
and expr_7 = (ctx: ClosureContext.t, input) =>
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
and expr_8 = (ctx: ClosureContext.t, input) =>
  {
    let child_ctx = ctx |> ClosureContext.child;

    choice([
      closure(child_ctx, expr_0)
      >@= (((_, _, cursor)) => ClosureContext.save(cursor, child_ctx)),
      expr_0(ctx) |> group,
      term(ctx),
    ]);
  }(
    input,
  )

/* 2, foo, <bar /> */
and term = (ctx: ClosureContext.t, input) =>
  choice([primitive, identifier(ctx), jsx(ctx, (expr_4, expr_0))], input);

let parser = expr_0;
