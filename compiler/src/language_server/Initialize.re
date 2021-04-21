open Kore;
open Deserialize;
open Yojson.Basic.Util;

module Compiler = Compile.Compiler;

type client_info_t = {
  name: string,
  version: option(string),
};

type trace_t =
  | Off
  | Message
  | Verbose;

type workspace_folder_t = {
  uri: string,
  name: string,
};

type params_t = {
  process_id: option(int),
  client_info: option(client_info_t),
  locale: option(string),
  root_uri: option(string),
  capabilities: option(Capabilities.t),
  trace: option(trace_t),
  workspace_folders: option(list(workspace_folder_t)),
};

let request =
  request(json => {
    let process_id = json |> member("processId") |> to_int_option;
    let locale = json |> member("locale") |> to_string_option;

    let root_uri = json |> member("rootUri") |> to_string_option;
    let root_path = json |> member("rootPath") |> to_string_option;
    let root_uri =
      switch (root_uri, root_path) {
      | (Some(_) as x, _)
      | (_, Some(_) as x) => x
      | _ => None
      };

    let trace =
      json
      |> member("trace")
      |> (
        fun
        | `String("off") => Some(Off)
        | `String("message") => Some(Message)
        | `String("verbose") => Some(Verbose)
        | _ => None
      );

    let client_info =
      json
      |> member("clientInfo")
      |> (
        fun
        | `Assoc(_) as x => {
            let name = x |> member("name") |> to_string;
            let version = x |> member("version") |> to_string_option;

            Some({name, version});
          }
        | _ => None
      );

    let workspace_folders =
      json
      |> member("workspaceFolders")
      |> (
        fun
        | `List(xs) =>
          xs
          |> List.map(x => {
               let uri = x |> member("uri") |> to_string;
               let name = x |> member("name") |> to_string;

               {uri, name};
             })
          |> Option.some

        | _ => None
      );

    {
      process_id,
      client_info,
      locale,
      root_uri,
      capabilities: None,
      trace,
      workspace_folders,
    };
  });

let handler =
    (
      {find_config, compilers}: Runtime.t,
      {params: {workspace_folders: folders}}: request_t(params_t),
    ) =>
  /* TODO: handle the case where workspace folders are nested? */
  folders
  |?: []
  |> List.iter(({name, uri}) => {
       let path = uri_to_path(uri);
       Log.info("creating compiler for '%s' project (%s)", name, path);

       let config = find_config(path);
       let root_dir =
         (
           Filename.is_relative(config.root_dir)
             ? Filename.concat(path, config.root_dir) : config.root_dir
         )
         |> Filename.resolve;

       let compiler =
         Compiler.create(
           ~report=_ => Protocol.report,
           {root_dir, source_dir: config.source_dir, name, fail_fast: false},
         );

       Hashtbl.add(
         compilers,
         name,
         {uri, compiler, contexts: Hashtbl.create(10)},
       );
     });
