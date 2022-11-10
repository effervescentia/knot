open Knot.Kore;
open AST;

let to_xml:
  (
    Type.t => string,
    list((Result.identifier_t, list(Result.node_t(TypeExpression.raw_t))))
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
