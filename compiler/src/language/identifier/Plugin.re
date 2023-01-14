open Knot.Kore;

include AST.Framework.Expression.Make({
  include Interface.Plugin;

  let parse = Parser.parse;

  let analyze = Analyzer.analyze;

  let format = (_, _, ppf) => Fmt.string(ppf);

  let to_xml = (_, name) =>
    Fmt.Node("Identifier", [("name", name |> ~@Fmt.string)], []);
});
