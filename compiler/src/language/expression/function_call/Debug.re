open Knot.Kore;

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  (expr_to_xml, (function_, arguments)) =>
    Fmt.Node(
      "FunctionCall",
      [],
      [
        Node("Function", [], [expr_to_xml(function_)]),
        Node("Arguments", [], arguments |> List.map(expr_to_xml)),
      ],
    );
