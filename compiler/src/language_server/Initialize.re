open Kore;

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

let method_key = "initialize";

let deserialize =
  JSON.Util.(
    json => {
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
                 let uri = Deserialize.uri(x);
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
    }
  );

let response = (name: string, workspace_support: bool) =>
  `Assoc([
    ("serverInfo", `Assoc([("name", `String(name))])),
    (
      "capabilities",
      `Assoc([
        (
          "workspace",
          `Assoc([
            /* support workspace folders */
            (
              "workspaceFolders",
              `Assoc([("support", `Bool(workspace_support))]),
            ),
          ]),
        ),
        /* enable hover support */
        ("hoverProvider", `Bool(true)),
        /* enable code completion support */
        (
          "completionProvider",
          `Assoc([
            ("resolveProvider", `Bool(true)),
            ("triggerCharacters", `List([`String(".")])),
          ]),
        ),
        /* enable go-to definition support */
        ("definitionProvider", `Bool(true)),
        /* enable document-scoped symbol support */
        ("documentSymbolProvider", `Bool(true)),
        /* enable workspace-scoped symbol support */
        ("workspaceSymbolProvider", `Bool(true)),
        /* enable formatting support */
        ("documentFormattingProvider", `Bool(true)),
        (
          "textDocumentSync",
          `Assoc([
            /* send notifications when files opened or closed */
            ("openClose", `Bool(true)),
            /* TODO: add incremental supports */
            /* send full documents when syncing */
            ("change", `Int(1)),
          ]),
        ),
      ]),
    ),
  ]);

let handler: Runtime.request_handler_t(params_t) =
  (
    {find_config, compilers} as runtime,
    {workspace_folders: folders, capabilities},
  ) => {
    /* TODO: handle the case where workspace folders are nested? */
    folders
    |?: []
    |> List.iter(({name, uri}) => {
         Log.info("creating compiler for '%s' project (%s)", name, uri);

         let config = find_config(uri);
         let root_dir =
           Filename.(
             (
               is_relative(config.root_dir)
                 ? concat(uri, config.root_dir) : config.root_dir
             )
             |> resolve
           );

         let compiler =
           Compiler.create(
             ~report=
               _ =>
                 Diagnostics.send(
                   runtime,
                   Filename.concat(root_dir, config.source_dir),
                 ),
             {
               root_dir,
               source_dir: config.source_dir,
               name,
               fail_fast: false,
               log_imports: false,
             },
           );

         Hashtbl.add(
           compilers,
           name,
           {uri, compiler, contexts: Hashtbl.create(10)},
         );
       });

    response(
      Target.knot,
      capabilities
      |?< (x => x.workspace)
      |?< (x => x.workspace_folders)
      |?: false,
    )
    |> Result.ok;
  };
