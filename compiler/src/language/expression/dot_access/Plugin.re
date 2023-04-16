let validate = Validator.validate;

include Interface;
include AST.Framework.Expression.Make({
  include Plugin;

  let parse = Parser.parse;

  let analyze = Analyzer.analyze;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});

let tokenize: Plugin.tokenize_t('expr, 'prim, 'typ) =
  (tokenize_expr, (expression, _)) =>
    expression
    |> tokenize_expr
    |> AST.TokenTree2.wrap(Knot.Node.get_range(expression));
