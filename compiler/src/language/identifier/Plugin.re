open Knot.Kore;
open AST;

let parse_id = Parser.identifier;

let analyze = Analyzer.analyze_identifier;

let pp = Fmt.string;

include Framework.Expression({
  type parse_arg_t = ParseContext.t;

  type value_t('a) = string;

  let parse = Parser.id_expression;

  let format = _ => pp;

  let to_xml = (_, name) =>
    Fmt.Node("Identifier", [("name", name |> ~@pp)], []);
});
