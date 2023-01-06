open Knot.Kore;

let to_xml = module_statement =>
  Dump.node_to_xml(
    ~unpack=
      AST.ModuleStatement.(
        fun
        | StdlibImport(named_imports) =>
          KImport.Debug.stdlib_import_to_xml(named_imports)

        | Import(namespace, main_import, named_imports) =>
          (namespace, main_import, named_imports) |> KImport.Plugin.to_xml

        | Export(kind, name, declaration) =>
          Fmt.Node(
            "Export",
            [("kind", ExportKind.to_string(kind))],
            [
              Dump.identifier_to_xml("Name", name),
              KDeclaration.Plugin.to_xml(~@AST.Type.pp, declaration),
            ],
          )
      )
      % List.single,
    "ModuleStatement",
    module_statement,
  );
