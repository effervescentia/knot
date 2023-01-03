open Knot.Kore;
open AST;

let pp = Formatter.format;

let analyze = Analyzer.analyze;

include Framework.Expression({
  type parse_arg_t = unit;

  type value_t('expr, 'typ) = Primitive.t;

  let parse = Parser.parse;

  let format = _ => pp;

  let to_xml = _ => Debug.to_xml;
});
