open Knot.Kore;
open AST;

let decoration_to_xml: Interface.decorator_t('a) => Fmt.xml_t(string) =
  decorator =>
    Dump.node_to_xml(
      ~unpack=
        ((name, arguments)) =>
          [
            Dump.identifier_to_xml("Name", name),
            ...arguments
               |> List.map(
                    Dump.node_to_xml(
                      ~dump_value=
                        prim =>
                          prim |> ~@KPrimitive.Formatter.format_primitive,
                      "Argument",
                    ),
                  ),
          ],
      "Decoration",
      decorator,
    );

let decorator_to_xml = ((name, parameters, target)) =>
  Fmt.Node(
    "Decorator",
    [],
    [
      Dump.identifier_to_xml("Name", name),
      Node(
        "Parameters",
        [],
        parameters |> List.map(KTypeExpression.Plugin.to_xml),
      ),
    ],
  );

let module_to_xml = ((name, statements, decorators)) =>
  Fmt.Node(
    "Module",
    [],
    [Dump.identifier_to_xml("Name", name)]
    @ (statements |> List.map(KTypeStatement.Plugin.to_xml))
    @ (decorators |> List.map(decoration_to_xml)),
  );

let to_xml: Interface.node_t => Fmt.xml_t(string) =
  Dump.node_to_xml(
    ~unpack=
      Interface.fold(~decorator=decorator_to_xml, ~module_=module_to_xml)
      % List.single,
    "Entry",
  );

// let to_xml: TypeDefinition.t => Fmt.xml_t(string) =
//   entries => Node("TypeDefinition", [], entries |> List.map(module_to_xml));
