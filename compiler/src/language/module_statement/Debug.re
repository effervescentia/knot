open Kore;
open Interface;

let to_xml:
  ('decl => Pretty.XML.xml_t(string), AST.Common.raw_t(t('decl))) =>
  Fmt.xml_t(string) =
  (decl_to_xml, module_statement) =>
    Dump.node_to_xml(
      ~unpack=
        fold(
          ~stdlib_import=Import.stdlib_import_to_xml,
          ~import=Import.to_xml,
          ~export=((kind, name, declaration)) =>
          Fmt.Node(
            "Export",
            [("kind", ExportKind.to_string(kind))],
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
