open Knot.Kore;

module Namespace = Reference.Namespace;

let to_xml: ((Namespace.t, list(AST.Module.import_t))) => Fmt.xml_t(string) =
  ((namespace, imports)) =>
    Node(
      "Import",
      [("namespace", namespace |> ~@Namespace.pp)],
      imports
      |> List.map(
           Dump.node_to_xml(
             ~unpack=
               AST.Module.(
                 fun
                 | MainImport(name) =>
                   Fmt.Node(
                     "MainImport",
                     [],
                     [Dump.node_to_xml(~dump_value=Fun.id, "Name", name)],
                   )
                 | NamedImport(name, alias) =>
                   Fmt.Node(
                     "NamedImport",
                     [],
                     [
                       Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
                       ...alias
                          |> Option.map(alias' =>
                               [
                                 Dump.node_to_xml(
                                   ~dump_value=Fun.id,
                                   "Alias",
                                   alias',
                                 ),
                               ]
                             )
                          |?: [],
                     ],
                   )
               )
               % (x => [x]),
             "ImportTarget",
           ),
         ),
    );
