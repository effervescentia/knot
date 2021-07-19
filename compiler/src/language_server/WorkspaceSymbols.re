open Kore;
open Deserialize;
open Yojson.Basic.Util;

type params_t = {
  query: string,
  partial_result_token: option(progress_token),
};

type symbol_info_t = {
  name: string,
  uri: string,
  kind: Capabilities.symbol_t,
  range: Cursor.range_t,
};

let request =
  request(json => {
    let query = json |> member("query") |> to_string;

    {query, partial_result_token: None};
  });

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
  )
  |> Response.wrap;

let handler = (runtime: Runtime.t, req: request_t(params_t)) => {
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
                   AST.(
                     fun
                     | Declaration(
                         MainExport(name) | NamedExport(name),
                         decl,
                       ) => {
                         let uri =
                           Filename.concat(
                             uri,
                             namespace
                             |> Namespace.to_path(compiler.config.source_dir),
                           );
                         let range = name |> Block.cursor |> Cursor.expand;
                         let name =
                           name |> Block.value |> Identifier.to_string;

                         switch (decl) {
                         | Constant(expr) =>
                           Some({
                             uri,
                             name,
                             range,
                             kind: Capabilities.Variable,
                           })
                         /* TODO: add type handling */
                         | Type(_) => None
                         | Function(args, expr) =>
                           Some({
                             uri,
                             name,
                             range,
                             kind: Capabilities.Function,
                           })
                         };
                       }
                     | Import(_) => None
                   ),
                 )
            )
         |> List.flatten
       )
    |> List.flatten;

  response(symbols) |> Protocol.reply(req);
};
