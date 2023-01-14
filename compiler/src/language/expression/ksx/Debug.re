open Knot.Kore;

let attribute_to_xml = (key, (name, value)) =>
  Fmt.Node(
    key,
    [],
    [
      Dump.identifier_to_xml("Name", name),
      ...value |> Option.map(Dump.node_to_xml("Value") % List.single) |?: [],
    ],
  );

let attribute_list_to_xml = attributes =>
  attributes
  |> List.map(
       Dump.node_to_xml(
         ~unpack=attribute_to_xml("Property") % List.single,
         "Attribute",
       ),
     );

let style_list_to_xml = (expr_to_xml, styles) =>
  List.is_empty(styles)
    ? [] : [Fmt.Node("Styles", [], styles |> List.map(expr_to_xml))];

let rec to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  ((expr_to_xml, dump_type) as arg, ksx) =>
    Node(
      "KSX",
      [],
      [
        switch (ksx) {
        | Tag(kind, view, styles, attributes, children) =>
          Fmt.Node(
            "Tag",
            [("kind", Interface.ViewKind.to_string(kind))],
            [Dump.node_to_xml(~dump_type, ~dump_value=Fun.id, "Name", view)]
            @ (styles |> style_list_to_xml(expr_to_xml))
            @ attribute_list_to_xml(attributes)
            @ (children |> children_to_xml(arg)),
          )

        | Fragment(children) =>
          Fmt.Node("Fragment", [], children |> children_to_xml(arg))
        },
      ],
    )
and children_to_xml = ((expr_to_xml, dump_type) as arg) =>
  List.map(
    Dump.node_to_xml(
      ~unpack=
        Interface.Child.(
          fun
          | Text(text) => Fmt.Node("Text", [("value", text)], [])
          | Node(node) => Fmt.Node("Node", [], [to_xml(arg, node)])
          | InlineExpression(expression) =>
            Fmt.Node("InlineExpression", [], [expr_to_xml(expression)])
        )
        % List.single,
      "Child",
    ),
  );
