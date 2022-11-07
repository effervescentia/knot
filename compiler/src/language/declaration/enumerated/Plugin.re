open Knot.Kore;

let parse = Parser.enumerated;

let pp = Formatter.pp_enumerated;

let to_xml:
  (
    AST.Type.t => string,
    list(
      (
        AST.Result.identifier_t,
        list(AST.Result.node_t(AST.TypeExpression.raw_t)),
      ),
    )
  ) =>
  Fmt.xml_t(string) =
  (dump_type, variants) =>
    Node(
      "Enumerated",
      [],
      variants
      |> List.map(((name, parameters)) =>
           Fmt.Node(
             "Variant",
             [],
             [
               Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
               ...parameters
                  |> List.map(
                       Dump.node_to_xml(
                         ~dump_type,
                         ~unpack=
                           parameter =>
                             [KTypeExpression.Plugin.to_xml_raw(parameter)],
                         "Parameter",
                       ),
                     ),
             ],
           )
         ),
    );
