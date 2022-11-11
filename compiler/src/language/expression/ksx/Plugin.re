open Knot.Kore;
open AST;

let analyze = Analyzer.analyze_root;

let pp = Formatter.pp_ksx;

include Framework.Expression({
  type parse_arg_t = (ParseContext.t, Parser.expression_parsers_arg_t);

  type value_t('a) = AST.Expression.jsx_t('a);

  let parse = Parser.ksx;

  let format = pp;

  let to_xml = Debug.to_xml;
});
