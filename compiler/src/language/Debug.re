open Kore;

let named_import_to_xml =
  Dump.node_to_xml(
    ~unpack=
      ((name, alias)) =>
        [
          Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
          ...alias
             |> Option.map(x =>
                  [Dump.node_to_xml(~dump_value=Fun.id, "Alias", x)]
                )
             |?: [],
        ],
    "NamedImport",
  );

let module_statement_to_xml = module_statement =>
  Dump.node_to_xml(
    ~unpack=
      AST.Module.(
        fun
        | StdlibImport(named_imports) => [
            Node(
              "StdlibImport",
              [],
              named_imports |> List.map(named_import_to_xml),
            ),
          ]

        | Import(namespace, main_import, named_imports) => [
            Node(
              "Import",
              [],
              (main_import |> Option.map(_ => []) |?: [])
              @ (named_imports |> List.map(named_import_to_xml)),
            ),
          ]

        | Export(kind, name, declaration) => [
            Node(
              "Export",
              [
                (
                  "kind",
                  switch (kind) {
                  | Main => "Main"
                  | Named => "Named"
                  },
                ),
              ],
              [Dump.node_to_xml(~dump_value=Fun.id, "Name", name)],
            ),
          ]
      ),
    "ModuleStatement",
    module_statement,
  );
