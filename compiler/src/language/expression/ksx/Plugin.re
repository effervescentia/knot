open Knot.Kore;
open AST;

let pp = Formatter.pp_ksx;

let analyze = Analyzer.analyze_root;

include Framework.Expression({
  type parse_arg_t = (ParseContext.t, Parser.expression_parsers_arg_t);

  type value_t('a) = AST.Expression.jsx_t('a);

  let parse = Parser.ksx;

  let format = Formatter.pp_ksx;

  let to_xml = Debug.to_xml;
});
