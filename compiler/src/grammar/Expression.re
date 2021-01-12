open Kore;

let primitive = Primitive.parser >|= AST.of_prim;

let identifier = M.identifier >@ Type.K_Invalid >|= AST.of_id;

let jsx = x => JSX.parser(x) >|= AST.of_jsx;

let group = x =>
  M.between(Character.open_paren, Character.close_paren, x) >|= AST.of_group;

let closure = x =>
  Statement.parser(x)
  |> many
  |> M.between(Character.open_brace, Character.close_brace)
  >|= AST.of_closure;

/**
 * each expression has a precedence denoted by its suffix
 *
 * the parser with the highest precedence should be matched first
 */

let rec expr_0 = input => chainl1(expr_1, Operator.logical_or, input)
and expr_1 = input => chainl1(expr_2, Operator.logical_and, input)
and expr_2 = input =>
  chainl1(expr_3, Operator.equality <|> Operator.inequality, input)
and expr_3 = input =>
  chainl1(
    expr_4,
    choice([
      Operator.less_or_eql,
      Operator.less_than,
      Operator.greater_or_eql,
      Operator.greater_than,
    ]),
    input,
  )
and expr_4 = input => chainl1(expr_5, Operator.add <|> Operator.sub, input)
and expr_5 = input => chainl1(expr_6, Operator.mult <|> Operator.div, input)
and expr_6 = input => chainr1(expr_7, Operator.expo, input)
and expr_7 = input =>
  M.unary_op(
    expr_8,
    choice([Operator.not_, Operator.positive, Operator.negative]),
    input,
  )
and expr_8 = input => (closure(expr_0) <|> group(expr_0) <|> value)(input)
and value = input => choice([primitive, identifier, jsx(expr_0)], input);

let parser = expr_0;