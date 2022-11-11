open Knot.Kore;
open AST;

let attribute_to_xml = (key, (name, value)) =>
  Fmt.Node(
    key,
    [],
    [
      Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
      ...value
         |> Option.map(value' => [Dump.node_to_xml("Value", value')])
         |?: [],
    ],
  );

let attribute_list_to_xml = attributes =>
  attributes
  |> List.map(
       Dump.node_to_xml(
         ~unpack=attribute_to_xml("Property") % (x => [x]),
         "Attribute",
       ),
     );

let rec to_xml:
  (
    (Expression.expression_t('a) => Fmt.xml_t(string), 'a => string),
    Expression.jsx_t('a)
  ) =>
  Fmt.xml_t(string) =
  ((expr_to_xml, dump_type), ksx) =>
    Node(
      "KSX",
      [],
      [
        switch (ksx) {
        | Tag(name, attributes, children) =>
          Fmt.Node(
            "Tag",
            [],
            [Dump.node_to_xml(~dump_value=Fun.id, "Name", name)]
            @ attribute_list_to_xml(attributes)
            @ (children |> children_to_xml(expr_to_xml, dump_type)),
          )
        | Component(name, attributes, children) =>
          Fmt.Node(
            "Component",
            [],
            [Dump.node_to_xml(~dump_type, ~dump_value=Fun.id, "Name", name)]
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
        Expression.(
          fun
          | Text(text) => Fmt.Node("Text", [("value", text)], [])
          | Node(node) =>
            Fmt.Node("Node", [], [to_xml((expr_to_xml, dump_type), node)])
          | InlineExpression(expr) =>
            Fmt.Node("InlineExpression", [], [expr_to_xml(expr)])
        )
        % (x => [x]),
      "Child",
    ),
  );
