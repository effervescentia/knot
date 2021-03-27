open Kore;

let _wrap_typed_lexeme = (f, (_, type_, cursor) as lexeme) => (
  lexeme |> f,
  type_,
  cursor,
);

let primitive =
  Primitive.parser
  >|= (((_, type_, cursor) as prim) => (prim |> AST.of_prim, type_, cursor));

let identifier = (scope: Scope.t) =>
  Identifier.parser
  >|= (
    ((id, cursor) as id_lexeme) => (
      id_lexeme |> AST.of_id,
      scope |> Scope.find(id),
      cursor,
    )
  );

let jsx = (scope: Scope.t, x) =>
  JSX.parser(scope, x)
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

let closure = (scope: Scope.t, x) =>
  Statement.parser(scope, x)
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
let rec expr_0 = (scope, input) =>
  chainl1(expr_1(scope), Operator.logical_or, input)
/* && */
and expr_1 = (scope, input) =>
  chainl1(expr_2(scope), Operator.logical_and, input)
/* ==, != */
and expr_2 = (scope, input) =>
  chainl1(expr_3(scope), Operator.equality <|> Operator.inequality, input)
/* <=, <, >=, > */
and expr_3 = (scope, input) =>
  chainl1(
    expr_4(scope),
    choice([
      Operator.less_or_eql,
      Operator.less_than,
      Operator.greater_or_eql,
      Operator.greater_than,
    ]),
    input,
  )
/* +, - */
and expr_4 = (scope, input) =>
  chainl1(expr_5(scope), Operator.add <|> Operator.sub, input)
/* *, / */
and expr_5 = (scope, input) =>
  chainl1(expr_6(scope), Operator.mult <|> Operator.div, input)
/* ^ */
and expr_6 = (scope, input) => chainr1(expr_7(scope), Operator.expo, input)
/* !, +, - */
and expr_7 = (scope, input) =>
  M.unary_op(
    expr_8(scope),
    choice([Operator.not, Operator.positive, Operator.negative]),
    input,
  )
/* {}, () */
and expr_8 = (scope, input) =>
  (
    closure(scope |> Scope.clone, expr_0)
    <|> (expr_0(scope) |> group)
    <|> term(scope)
  )(
    input,
  )
/* 2, foo, <bar /> */
and term = (scope, input) =>
  choice([primitive, identifier(scope), jsx(scope, expr_0)], input);

let parser = expr_0;
