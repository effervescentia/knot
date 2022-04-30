open Kore;
open Deserialize;

type params_t = {
  query: string,
  partial_result_token: option(progress_token),
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
    |> List.map(({name, uri, kind, range}) =>
         `Assoc([
           ("name", `String(name)),
           ("kind", `Int(kind |> Response.symbol)),
           (
             "location",
             `Assoc([
               ("uri", `String(file_schema ++ uri)),
               ("range", range |> Response.range),
             ]),
           ),
         ])
       ),
  );

let handler: Runtime.request_handler_t(params_t) =
  (runtime, params) => {
    let symbols =
      runtime.compilers
      |> Hashtbl.to_seq_values
      |> List.of_seq
      |> List.map((Runtime.{uri, compiler}) =>
           compiler.modules
           |> Hashtbl.to_seq
           |> List.of_seq
           |> List.map(((namespace, ModuleTable.{ast})) =>
                ast
                |> List.filter_map(
                     Node.Raw.get_value
                     % (
                       fun
                       | AST.Declaration(
                           MainExport(name) | NamedExport(name),
                           decl,
                         ) => {
                           let uri =
                             Filename.concat(
                               uri,
                               namespace
                               |> Namespace.to_path(
                                    compiler.config.source_dir,
                                  ),
                             );
                           let range = Node.Raw.get_range(name);
                           let name =
                             name |> Node.Raw.get_value |> ~@Identifier.pp;

                           Some(
                             switch (Node.get_value(decl)) {
                             | Constant(expr) => {
                                 uri,
                                 name,
                                 range,
                                 kind: Capabilities.Variable,
                               }
                             | Function(args, expr) => {
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
              )
           |> List.flatten
         )
      |> List.flatten;

    Result.ok(`Null);
  };
