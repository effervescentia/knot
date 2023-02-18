open Knot.Kore;

include AST.Framework.Expression.Make({
  include Interface.Plugin;

  let parse = Parser.parse;

  let analyze = Analyzer.analyze;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});

let tokenize: Interface.Plugin.tokenize_t('expr, 'prim, 'typ) =
  (tokenize_expr, rules) =>
    rules |> List.map(fst % snd % tokenize_expr) |> AST.TokenTree2.of_list;
