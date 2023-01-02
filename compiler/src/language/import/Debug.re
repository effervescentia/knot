open Knot.Kore;

module Namespace = Reference.Namespace;

let main_import_to_xml = Dump.node_to_xml(~dump_value=Fun.id, "MainImport");

let named_import_to_xml =
  Dump.node_to_xml(
    ~unpack=
      ((name, alias)) =>
        [
          Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
          ...alias
             |> Option.map(alias' =>
                  [Dump.node_to_xml(~dump_value=Fun.id, "Alias", alias')]
                )
             |?: [],
        ],
    "NamedImport",
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
      (
        main_import
        |> Option.map(main_import' => [main_import_to_xml(main_import')])
        |?: []
      )
      @ (named_imports |> List.map(named_import_to_xml)),
    );
