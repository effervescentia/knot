open Knot.Kore;

let argument_to_xml:
  (
    AST.Result.expression_t => Fmt.xml_t(string),
    AST.Type.t => string,
    AST.Result.argument_t
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, dump_type, parameter) =>
    Dump.node_to_xml(
      ~dump_type,
      ~unpack=
        (AST.Expression.{name, default, type_}) =>
          [Dump.node_to_xml(~dump_value=Fun.id, "Name", name)]
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
                 [
                   Fmt.Node(
                     "Type",
                     [],
                     [KTypeExpression.Plugin.to_xml(type_')],
                   ),
                 ]
               )
            |?: []
          ),
      "Parameter",
      parameter,
    );

let argument_list_to_xml:
  (
    AST.Result.expression_t => Fmt.xml_t(string),
    AST.Type.t => string,
    list(AST.Result.argument_t)
  ) =>
  list(Fmt.xml_t(string)) =
  (expr_to_xml, dump_type, parameters) =>
    parameters |> List.map(argument_to_xml(expr_to_xml, dump_type));
