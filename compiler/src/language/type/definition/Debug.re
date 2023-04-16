open AST;
open Kore;

let decoration_to_xml: Interface.decorator_t => Fmt.xml_t(string) =
  KDecorator.Plugin.to_xml((
    Dump.node_to_xml(
      ~unpack=Primitive.primitive_to_xml % List.single,
      "Value",
    ),
    ~@Type.pp,
  ));

let decorator_to_xml = ((name, parameters, target)) =>
  Fmt.Node(
    "Decorator",
    [],
    [
      Dump.identifier_to_xml("Name", name),
      Node("Parameters", [], parameters |> List.map(TypeExpression.to_xml)),
    ],
  );

let module_to_xml = ((name, statements, decorators)) =>
  Fmt.Node(
    "Module",
    [],
    [Dump.identifier_to_xml("Name", name)]
    @ (statements |> List.map(TypeStatement.to_xml))
    @ (decorators |> List.map(decoration_to_xml)),
  );

let to_xml: Interface.node_t => Fmt.xml_t(string) =
  Dump.node_to_xml(
    ~unpack=
      Interface.fold(~decorator=decorator_to_xml, ~module_=module_to_xml)
      % List.single,
    "Entry",
  );
