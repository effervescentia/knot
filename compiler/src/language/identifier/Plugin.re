open Knot.Kore;
open AST;

let analyze = Analyzer.analyze;

let pp = Fmt.string;

include Framework.Expression({
  type parse_arg_t = ParseContext.t;

  type value_t('a) = string;

  let parse = Parser.parse;

  let format = _ => pp;

  let to_xml = (_, name) =>
    Fmt.Node("Identifier", [("name", name |> ~@pp)], []);
});
