open Knot.Kore;

let parse = Parser.import;

let pp = Formatter.pp_import;

let to_xml:
  ((Reference.Namespace.t, list(AST.Result.import_t))) => Fmt.xml_t(string) =
  ((namespace, imports)) =>
    Node(
      "Import",
      [("namespace", namespace |> ~@Reference.Namespace.pp)],
      imports
      |> List.map(
           Dump.node_to_xml(
             ~unpack=
               (
                 fun
                 | AST.Result.MainImport(name) =>
                   Fmt.Node(
                     "MainImport",
                     [],
                     [Dump.node_to_xml(~dump_value=Fun.id, "Name", name)],
                   )
                 | AST.Result.NamedImport(name, alias) =>
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
