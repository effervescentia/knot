open Infix;

let range_attribute = (node: Node.t('a, 'b)) => (
  "range",
  node |> Node.get_range |> ~@Range.pp,
);

let type_attribute = (dump_type: option('b => string), node: Node.t('a, 'b)) =>
  dump_type
  |> Option.map(dump_type' =>
       [("type", node |> Node.get_type |> dump_type')]
     )
  |?: [];

let node_to_xml =
  Pretty.XML.(
    (
      ~dump_value: option('a => string)=?,
      ~dump_type: option('b => string)=?,
      ~unpack: 'a => list(xml_t(string))=_ => [],
      name: string,
      node: Node.t('a, 'b),
    ) =>
      Node(
        name,
        [range_attribute(node)]
        @ type_attribute(dump_type, node)
        @ (
          dump_value
          |> Option.map(dump_value' =>
               [("value", node |> fst |> dump_value')]
             )
          |?: []
        ),
        node |> fst |> unpack,
      )
  );
