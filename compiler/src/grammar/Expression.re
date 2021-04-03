open Kore;

let _wrap_typed_lexeme = (f, (_, type_, cursor) as lexeme) => (
  lexeme |> f,
  type_,
  cursor,
);

let primitive =
  Primitive.parser
  >|= (((_, type_, cursor) as prim) => (prim |> AST.of_prim, type_, cursor));

let identifier = (ctx: Context.t) =>
  Identifier.parser
  >|= (
    ((id, cursor) as id_lexeme) => (
      id_lexeme |> AST.of_id,
      ctx.scope |> Scope.find(id),
      cursor,
    )
  );

let jsx = (ctx: Context.t, x) =>
  JSX.parser(ctx, x)
  >|= (
    ((_, cursor) as jsx) => (
      jsx |> AST.of_jsx,
      Type.K_Strong(K_Element),
      cursor,
    )
  );

let group = x =>
  M.between(Symbol.open_group, Symbol.close_group, x)
  >|= (
    block => {
      let (_, type_, _) as group = block |> Block.value;

      (group |> AST.of_group, type_, block |> Block.cursor);
    }
  );

let closure = (ctx: Context.t, x) =>
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
          | None => Type.K_Strong(K_Nil)
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
let rec expr_0 = (ctx: Context.t, input) =>
  chainl1(expr_1(ctx), Operator.logical_or, input)
/* && */
and expr_1 = (ctx: Context.t, input) =>
  chainl1(expr_2(ctx), Operator.logical_and, input)
/* ==, != */
and expr_2 = (ctx: Context.t, input) =>
  chainl1(expr_3(ctx), Operator.equality <|> Operator.inequality, input)
/* <=, <, >=, > */
and expr_3 = (ctx: Context.t, input) =>
  chainl1(
    expr_4(ctx),
    choice([
      Operator.less_or_eql,
      Operator.less_than,
      Operator.greater_or_eql,
      Operator.greater_than,
    ]),
    input,
  )
/* +, - */
and expr_4 = (ctx: Context.t, input) =>
  chainl1(expr_5(ctx), Operator.add <|> Operator.sub, input)
/* *, / */
and expr_5 = (ctx: Context.t, input) =>
  chainl1(expr_6(ctx), Operator.mult <|> Operator.div, input)
/* ^ */
and expr_6 = (ctx: Context.t, input) =>
  chainr1(expr_7(ctx), Operator.expo, input)
/* !, +, - */
and expr_7 = (ctx: Context.t, input) =>
  M.unary_op(
    expr_8(ctx),
    choice([Operator.not, Operator.positive, Operator.negative]),
    input,
  )
/* {}, () */
and expr_8 = (ctx: Context.t, input) =>
  (
    closure(ctx |> Context.clone, expr_0)
    <|> (expr_0(ctx) |> group)
    <|> term(ctx)
  )(
    input,
  )
/* 2, foo, <bar /> */
and term = (ctx: Context.t, input) =>
  choice([primitive, identifier(ctx), jsx(ctx, expr_0)], input);

let parser = expr_0;
