open Knot.Kore;

let to_xml:
  (
    'decl => Pretty.XML.xml_t(string),
    AST.Common.raw_t(Interface.t('decl))
  ) =>
  Fmt.xml_t(string) =
  (decl_to_xml, module_statement) =>
    Dump.node_to_xml(
      ~unpack=
        Interface.fold(
          ~stdlib_import=KImport.Debug.stdlib_import_to_xml,
          ~import=KImport.Plugin.to_xml,
          ~export=((kind, name, declaration)) =>
          Fmt.Node(
            "Export",
            [("kind", Interface.ExportKind.to_string(kind))],
            [
              Dump.identifier_to_xml("Name", name),
              decl_to_xml(declaration),
            ],
          )
        )
        % List.single,
      "ModuleStatement",
      module_statement,
    );
