open Knot.Kore;
open AST;

let variant_parameter_to_xml = dump_type =>
  Dump.node_to_xml(
    ~dump_type,
    ~unpack=KTypeExpression.Debug.to_xml_raw % List.single,
    "Parameter",
  );

let variant_to_xml = (dump_type, (name, parameters)) =>
  Fmt.Node(
    "Variant",
    [],
    [
      Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
      ...parameters |> List.map(variant_parameter_to_xml(dump_type)),
    ],
  );

let to_xml:
  (
    Type.t => string,
    list((Result.identifier_t, list(Result.node_t(TypeExpression.raw_t))))
  ) =>
  Fmt.xml_t(string) =
  (dump_type, variants) =>
    Node("Enumerated", [], variants |> List.map(variant_to_xml(dump_type)));
