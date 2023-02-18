open Knot.Kore;
open AST;

let entry_to_xml = (key, (name, expr)) =>
  Fmt.Node(
    key,
    [],
    [
      Dump.identifier_to_xml("Name", name),
      Node("Value", [], [KTypeExpression.Plugin.to_xml(expr)]),
    ],
  );

let enumerated_to_xml = ((name, variants)) =>
  Fmt.Node(
    "Enumerated",
    [],
    [
      Dump.identifier_to_xml("Name", name),
      Node(
        "Value",
        [],
        variants
        |> List.map(((name, parameters)) =>
             Fmt.Node(
               "Variant",
               [],
               [
                 Dump.identifier_to_xml("Name", name),
                 Node(
                   "Parameters",
                   [],
                   parameters |> List.map(KTypeExpression.Plugin.to_xml),
                 ),
               ],
             )
           ),
      ),
    ],
  );

let to_xml: Interface.node_t => Fmt.xml_t(string) =
  Dump.node_to_xml(
    ~unpack=
      Interface.fold(
        ~declaration=entry_to_xml("Declaration"),
        ~type_=entry_to_xml("Type"),
        ~enumerated=enumerated_to_xml,
      )
      % List.single,
    "Statement",
  );
