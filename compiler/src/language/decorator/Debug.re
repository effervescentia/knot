open Knot.Kore;

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  ((expr_to_xml, dump_type)) =>
    Dump.node_to_xml(
      ~unpack=
        ((name, parameters)) =>
          [
            Dump.node_to_xml(~dump_type, ~dump_value=Fun.id, "Name", name),
            Node("Parameters", [], parameters |> List.map(expr_to_xml)),
          ],
      "Decorator",
    );
