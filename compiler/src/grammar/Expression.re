open Kore;

let primitive = Primitive.parser >|= AST.of_prim;

let identifier = M.identifier >|= fst % AST.of_id;

/**
 * each expression has a precedence denoted by its suffix
 *
 * the parser with the highest precedence should be matched first
 */

let rec expr_0 = input =>
  chainl1(expr_1, Operator.logical_and <|> Operator.logical_or, input)
and expr_1 = input =>
  chainl1(expr_2, Operator.equality <|> Operator.inequality, input)
and expr_2 = input =>
  chainl1(
    expr_3,
    Operator.less_or_eql
    <|> Operator.less_than
    <|> Operator.greater_or_eql
    <|> Operator.greater_than,
    input,
  )
and expr_3 = input => chainl1(expr_4, Operator.add <|> Operator.sub, input)
and expr_4 = input => chainl1(value, Operator.mult <|> Operator.div, input)
and value = input => choice([primitive, identifier], input);

let parser = expr_0;
