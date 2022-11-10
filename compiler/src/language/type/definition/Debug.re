open Knot.Kore;
open AST;

let entry_to_xml = (key, (name, expr)) =>
  Fmt.Node(
    key,
    [],
    [
      Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
      Node("Value", [], [KTypeExpression.Plugin.to_xml(expr)]),
    ],
  );

let enumerated_to_xml = (name, variants) =>
  Fmt.Node(
    "Enumerated",
    [],
    [
      Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
      Node(
        "Value",
        [],
        variants
        |> List.map(((name, parameters)) =>
             Fmt.Node(
               "Variant",
               [],
               [
                 Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
                 Node(
                   "Parameters",
                   [],
                   parameters |> List.map(KTypeExpression.Plugin.to_xml),
                 ),
               ],
             )
           ),
      ),
    ],
  );

let decorator_to_xml: TypeDefinition.decorator_t('a) => Fmt.xml_t(string) =
  decorator =>
    Dump.node_to_xml(
      ~unpack=
        ((name, arguments)) =>
          [
            Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
            ...arguments
               |> List.map(
                    Dump.node_to_xml(
                      ~dump_value=
                        prim =>
                          prim
                          |> ~@
                               KPrimitive.Plugin.pp(_ =>
                                 raise(NotImplemented)
                               ),
                      "Argument",
                    ),
                  ),
          ],
      "Decorator",
      decorator,
    );

let module_to_xml: TypeDefinition.module_t => Fmt.xml_t(string) =
  Dump.node_to_xml(
    ~unpack=
      TypeDefinition.(
        fun
        | Decorator(name, parameters, target) => [
            Fmt.Node(
              "Decorator",
              [],
              [
                Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
                Node(
                  "Parameters",
                  [],
                  parameters |> List.map(KTypeExpression.Plugin.to_xml),
                ),
              ],
            ),
          ]
        | Module(name, statements, decorators) => [
            Fmt.Node(
              "Decorator",
              [],
              [Dump.node_to_xml(~dump_value=Fun.id, "Name", name)]
              @ (
                statements
                |> List.map(
                     Dump.node_to_xml(
                       ~unpack=
                         (
                           fun
                           | Declaration(name, expr) =>
                             entry_to_xml("Declaration", (name, expr))
                           | Type(name, expr) =>
                             entry_to_xml("Type", (name, expr))
                           | Enumerated(name, variants) =>
                             enumerated_to_xml(name, variants)
                         )
                         % (x => [x]),
                       "Statement",
                     ),
                   )
              )
              @ (decorators |> List.map(decorator_to_xml)),
            ),
          ]
      ),
    "Entry",
  );

let to_xml: TypeDefinition.t => Fmt.xml_t(string) =
  entries => Node("TypeDefinition", [], entries |> List.map(module_to_xml));
