open Kore;

type params_t = {
  query: string,
  partial_result_token: option(Protocol.progress_token),
};

type symbol_info_t = {
  name: string,
  uri: string,
  kind: Capabilities.symbol_t,
  range: Range.t,
};

let method_key = "workspace/symbol";

let deserialize =
  JSON.Util.(
    json => {
      let query = json |> member("query") |> to_string;

      {query, partial_result_token: None};
    }
  );

let response = (symbols: list(symbol_info_t)) =>
  `List(
    symbols
    |> List.map(({name, uri, kind, range, _}) =>
         `Assoc([
           ("name", `String(name)),
           ("kind", `Int(kind |> Serialize.symbol)),
           (
             "location",
             `Assoc([
               ("uri", `String(file_schema ++ uri)),
               ("range", range |> Serialize.range),
             ]),
           ),
         ])
       ),
  );

let handler: Runtime.request_handler_t(params_t) =
  ({compilers, _}, _) => {
    let symbols =
      compilers
      |> Hashtbl.to_seq_values
      |> List.of_seq
      |> List.map((Runtime.{uri, compiler, _}) =>
           compiler.modules
           |> ModuleTable.to_module_list
           |> List.map(
                Tuple.map_snd2(
                  ModuleTable.(
                    get_entry_data % Option.map(({ast, _}) => ast)
                  ),
                )
                % (
                  fun
                  | (namespace, Some(ast)) =>
                    ast
                    |> List.filter_map(
                         fst
                         % AST.ModuleStatement.(
                             fun
                             | Export(_, name, decl) => {
                                 let uri =
                                   Filename.concat(
                                     uri,
                                     namespace
                                     |> Namespace.to_path(
                                          compiler.config.source_dir
                                          |> Filename.relative_to(
                                               compiler.config.root_dir,
                                             ),
                                        ),
                                   );
                                 let range = Node.get_range(name);
                                 let name = fst(name);

                                 Some(
                                   switch (fst(decl)) {
                                   | Constant(_) => {
                                       uri,
                                       name,
                                       range,
                                       kind: Capabilities.Variable,
                                     }
                                   | Enumerated(_) => {
                                       uri,
                                       name,
                                       range,
                                       kind: Capabilities.Enum,
                                     }
                                   | Function(_) => {
                                       uri,
                                       name,
                                       range,
                                       kind: Capabilities.Function,
                                     }
                                   | View(_) => {
                                       uri,
                                       name,
                                       range,
                                       kind: Capabilities.Function,
                                     }
                                   },
                                 );
                               }
                             | _ => None
                           ),
                       )

                  | _ => []
                ),
              )
           |> List.flatten
         )
      |> List.flatten;

    ignore(symbols);

    Result.ok(`Null);
  };
