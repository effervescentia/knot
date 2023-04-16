open Kore;
open AST;

let parameter_to_xml:
  (
    Node.t('expr, 'typ) => Fmt.xml_t(string),
    'typ => string,
    Interface.Parameter.node_t('expr, 'typ)
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, dump_type, parameter) =>
    Dump.node_to_xml(
      ~dump_type,
      ~unpack=
        ((name, type_, default)) =>
          [Dump.identifier_to_xml("Name", name)]
          @ (
            default
            |> Option.map(default' =>
                 [Fmt.Node("Default", [], [expr_to_xml(default')])]
               )
            |?: []
          )
          @ (
            type_
            |> Option.map(type_' =>
                 [Fmt.Node("Type", [], [TypeExpression.to_xml(type_')])]
               )
            |?: []
          ),
      "Parameter",
      parameter,
    );

let parameter_list_to_xml:
  (
    Node.t('expr, 'typ) => Fmt.xml_t(string),
    'typ => string,
    list(Interface.Parameter.node_t('expr, 'typ))
  ) =>
  list(Fmt.xml_t(string)) =
  (expr_to_xml, dump_type, parameters) =>
    parameters |> List.map(parameter_to_xml(expr_to_xml, dump_type));
