open Knot.Kore;

let parse_style_literal = Parser.parse_style_literal;
let format_style_rule_list = Formatter.format_style_rule_list;
let validate_style_rule = Validator.validate_style_rule;

include Interface;
include AST.Framework.Expression.Make({
  include Plugin;

  let parse = Parser.parse;

  let analyze = Analyzer.analyze;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});

let tokenize: Plugin.tokenize_t('expr, 'prim, 'typ) =
  (tokenize_expr, rules) =>
    rules |> List.map(fst % snd % tokenize_expr) |> AST.TokenTree2.of_list;
