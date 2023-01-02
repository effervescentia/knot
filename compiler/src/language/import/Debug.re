open Knot.Kore;

module Namespace = Reference.Namespace;

let main_import_to_xml = Dump.identifier_to_xml("MainImport");

let named_import_to_xml =
  Dump.node_to_xml(
    ~unpack=
      ((name, alias)) =>
        [
          Dump.identifier_to_xml("Name", name),
          ...alias
             |> Option.map(Dump.identifier_to_xml("Alias") % List.single)
             |?: [],
        ],
    "NamedImport",
  );

let stdlib_import_to_xml = named_imports =>
  Fmt.Node(
    "StdlibImport",
    [],
    named_imports |> List.map(named_import_to_xml),
  );

let to_xml:
  (
    (
      Namespace.t,
      option(AST.Common.identifier_t),
      list(AST.Module.named_import_t),
    )
  ) =>
  Fmt.xml_t(string) =
  ((namespace, main_import, named_imports)) =>
    Node(
      "Import",
      [("namespace", namespace |> ~@Namespace.pp)],
      (main_import |> Option.map(main_import_to_xml % List.single) |?: [])
      @ (named_imports |> List.map(named_import_to_xml)),
    );
