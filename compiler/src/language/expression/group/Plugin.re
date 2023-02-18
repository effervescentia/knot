open Knot.Kore;

include AST.Framework.Expression.Make({
  include Interface.Plugin;

  let parse = Parser.parse;

  let analyze = Analyzer.analyze;

  let format = Formatter.format;

  let to_xml = ((expr_to_xml, _), expression) =>
    Fmt.Node("Group", [], [expr_to_xml(expression)]);
});

let tokenize: Interface.Plugin.tokenize_t('expr, 'prim, 'typ) =
  (tokenize_expr, expression) =>
    expression
    |> tokenize_expr
    |> AST.TokenTree2.wrap(Node.get_range(expression));
