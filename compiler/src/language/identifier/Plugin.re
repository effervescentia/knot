open Knot.Kore;
open AST;

let parse_id = Parser.identifier;

let analyze = Analyzer.analyze_identifier;

include Framework.Expression({
  type parse_arg_t = ParseContext.t;

  type value_t('a) = string;

  let parse = Parser.id_expression;

  let pp = _ => Fmt.string;

  let to_xml = (_, name) =>
    Fmt.Node("Identifier", [("name", name |> ~@pp())], []);
});
