open Knot.Kore;

module Namespace = Reference.Namespace;

let main_import_to_xml = name =>
  Fmt.Node(
    "MainImport",
    [],
    [Dump.node_to_xml(~dump_value=Fun.id, "Name", name)],
  );

let named_import_to_xml = (name, alias) =>
  Fmt.Node(
    "NamedImport",
    [],
    [
      Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
      ...alias
         |> Option.map(alias' =>
              [Dump.node_to_xml(~dump_value=Fun.id, "Alias", alias')]
            )
         |?: [],
    ],
  );

let import_to_xml =
  Dump.node_to_xml(
    ~unpack=
      AST.Module.(
        fun
        | MainImport(name) => main_import_to_xml(name)
        | NamedImport(name, alias) => named_import_to_xml(name, alias)
      )
      % (x => [x]),
    "ImportTarget",
  );

let to_xml: ((Namespace.t, list(AST.Module.import_t))) => Fmt.xml_t(string) =
  ((namespace, imports)) =>
    Node(
      "Import",
      [("namespace", namespace |> ~@Namespace.pp)],
      imports |> List.map(import_to_xml),
    );
