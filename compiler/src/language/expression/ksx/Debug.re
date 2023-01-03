open Knot.Kore;
open AST;

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

let rec to_xml:
  (
    (Node.t('expr, 'typ) => Fmt.xml_t(string), 'typ => string),
    AST.KSX.t('expr, 'typ)
  ) =>
  Fmt.xml_t(string) =
  ((expr_to_xml, dump_type), ksx) =>
    Node(
      "KSX",
      [],
      [
        switch (ksx) {
        | Tag(kind, view, styles, attributes, children) =>
          Fmt.Node(
            "Tag",
            [("kind", AST.KSX.ViewKind.to_string(kind))],
            [Dump.node_to_xml(~dump_type, ~dump_value=Fun.id, "Name", view)]
            @ (styles |> style_list_to_xml(expr_to_xml))
            @ attribute_list_to_xml(attributes)
            @ (children |> children_to_xml(expr_to_xml, dump_type)),
          )

        | Fragment(children) =>
          Fmt.Node(
            "Fragment",
            [],
            children |> children_to_xml(expr_to_xml, dump_type),
          )
        },
      ],
    )
and children_to_xml = (expr_to_xml, dump_type) =>
  List.map(
    Dump.node_to_xml(
      ~unpack=
        AST.KSX.Child.(
          fun
          | Text(text) => Fmt.Node("Text", [("value", text)], [])
          | Node(node) =>
            Fmt.Node("Node", [], [to_xml((expr_to_xml, dump_type), node)])
          | InlineExpression(expression) =>
            Fmt.Node("InlineExpression", [], [expr_to_xml(expression)])
        )
        % List.single,
      "Child",
    ),
  );
