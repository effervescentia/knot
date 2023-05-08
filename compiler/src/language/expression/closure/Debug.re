open Kore;

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  ((expr_to_xml, dump_type), statements) =>
    Fmt.Node(
      "Closure",
      [],
      statements |> List.map(Statement.to_xml((expr_to_xml, dump_type))),
    );
