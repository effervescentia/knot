open Knot.Kore;
open AST;

let argument_to_xml:
  (
    Result.expression_t => Fmt.xml_t(string),
    Type.t => string,
    Result.argument_t
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, dump_type, parameter) =>
    Dump.node_to_xml(
      ~dump_type,
      ~unpack=
        (Expression.{name, default, type_}) =>
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
    Result.expression_t => Fmt.xml_t(string),
    Type.t => string,
    list(Result.argument_t)
  ) =>
  list(Fmt.xml_t(string)) =
  (expr_to_xml, dump_type, parameters) =>
    parameters |> List.map(argument_to_xml(expr_to_xml, dump_type));
