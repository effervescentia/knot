open Knot.Kore;
open AST;

let _get_tag_name =
  Interface.fold(
    ~nil=() => "Nil",
    ~boolean=() => "Boolean",
    ~integer=() => "Integer",
    ~float=() => "Float",
    ~string=() => "String",
    ~element=() => "Element",
    ~style=() => "Style",
    ~identifier=_ => "Identifier",
    ~group=_ => "Group",
    ~list=_ => "List",
    ~object_=_ => "Object",
    ~function_=_ => "Function",
    ~dot_access=_ => "DotAccess",
    ~view=_ => "View",
  );

let rec to_xml_raw: Interface.t => Fmt.xml_t(string) =
  expr => Node(_get_tag_name(expr), [], _get_children(expr))

and to_xml: Interface.node_t => Fmt.xml_t(string) =
  expr =>
    Node(
      expr |> fst |> _get_tag_name,
      [Dump.range_attribute(expr)],
      expr |> fst |> _get_children,
    )

and required_to_xml = ((key, value)) =>
  Fmt.Node(
    "Required",
    [],
    [
      Dump.identifier_to_xml("Key", key),
      Node("Value", [], [to_xml(value)]),
    ],
  )

and optional_to_xml = ((key, value)) =>
  Fmt.Node(
    "Optional",
    [],
    [
      Dump.identifier_to_xml("Key", key),
      Node("Value", [], [to_xml(value)]),
    ],
  )

and spread_to_xml = value =>
  Fmt.Node("Spread", [], [Node("Value", [], [to_xml(value)])])

and object_entry_to_xml = entry =>
  entry
  |> Interface.ObjectEntry.fold(
       ~required=required_to_xml,
       ~optional=optional_to_xml,
       ~spread=spread_to_xml,
     )

and _get_children: Interface.t => list(Fmt.xml_t(string)) =
  fun
  | Identifier(name) => [Dump.identifier_to_xml("Name", name)]

  | Group(expr)
  | List(expr) => [to_xml(expr)]

  | Object(properties) => properties |> List.map(fst % object_entry_to_xml)

  | Function((parameters, result)) => [
      Node("Parameters", [], parameters |> List.map(to_xml)),
      Node("Result", [], [to_xml(result)]),
    ]

  | DotAccess((root, property)) => [
      Node("Root", [], [to_xml(root)]),
      Dump.identifier_to_xml("Property", property),
    ]

  | View((properties, result)) => [
      Node("Properties", [], [to_xml(properties)]),
      Node("Result", [], [to_xml(result)]),
    ]

  | _ => [];
