open Knot.Kore;
open AST;

let _get_tag_name: TypeExpression.raw_t => string =
  fun
  | Nil => "Nil"
  | Boolean => "Boolean"
  | Integer => "Integer"
  | Float => "Float"
  | String => "String"
  | Element => "Element"
  | Style => "Style"
  | Identifier(_) => "Identifier"
  | Group(_) => "Group"
  | List(_) => "List"
  | Object(_) => "Object"
  | Function(_) => "Function"
  | DotAccess(_) => "DotAccess"
  | View(_) => "View";

let rec to_xml_raw: TypeExpression.raw_t => Fmt.xml_t(string) =
  expr => Node(_get_tag_name(expr), [], _get_children(expr))

and to_xml: TypeExpression.t => Fmt.xml_t(string) =
  expr =>
    Node(
      expr |> fst |> _get_tag_name,
      [Dump.range_attribute(expr)],
      expr |> fst |> _get_children,
    )

and _get_children: TypeExpression.raw_t => list(Fmt.xml_t(string)) =
  fun
  | Identifier(name) => [Dump.node_to_xml(~dump_value=Fun.id, "Name", name)]
  | Group(expr) => [to_xml(expr)]
  | List(expr) => [to_xml(expr)]

  | Object(properties) =>
    properties
    |> List.map(
         fst
         % TypeExpression.(
             fun
             | Required(key, value) =>
               Fmt.Node(
                 "Required",
                 [],
                 [
                   Dump.node_to_xml(~dump_value=Fun.id, "Key", key),
                   Node("Value", [], [to_xml(value)]),
                 ],
               )
             | Optional(key, value) =>
               Fmt.Node(
                 "Optional",
                 [],
                 [
                   Dump.node_to_xml(~dump_value=Fun.id, "Key", key),
                   Node("Value", [], [to_xml(value)]),
                 ],
               )
             | Spread(value) =>
               Fmt.Node(
                 "Spread",
                 [],
                 [Node("Value", [], [to_xml(value)])],
               )
           ),
       )

  | Function(parameters, result) => [
      Node("Parameters", [], parameters |> List.map(to_xml)),
      Node("Result", [], [to_xml(result)]),
    ]

  | DotAccess(root, property) => [
      Node("Root", [], [to_xml(root)]),
      Dump.node_to_xml(~dump_value=Fun.id, "Property", property),
    ]

  | View(properties, result) => [
      Node("Properties", [], [to_xml(properties)]),
      Node("Result", [], [to_xml(result)]),
    ]

  | _ => [];
