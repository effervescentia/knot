open Kore;

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

type request_t('a) = {
  jsonrpc: string,
  id: int,
  method: string,
  params: 'a,
};

type notification_t('a) = {
  jsonrpc: string,
  method: string,
  params: 'a,
};

type initialize_params_t = {
  process_id: option(int),
  client_info: option(client_info_t),
  locale: option(string),
  root_uri: option(string),
  capabilities: option(Capabilities.t),
  trace: option(trace_t),
  workspace_folders: option(list(workspace_folder_t)),
};

type text_document_t = {uri: string};

type position_t = {
  line: int,
  character: int,
};

type hover_params_t = {
  text_document: text_document_t,
  position: position_t,
};

type text_document_item_t = {
  uri: string,
  language_id: string,
  version: int,
  text: string,
};

type file_open_params_t = {text_document: text_document_item_t};

type file_close_params_t = {text_document: text_document_t};

type progress_token =
  | Int(int)
  | String(string);

type go_to_definition_params_t = {
  text_document: text_document_t,
  position: position_t,
  partial_result_token: option(progress_token),
};

type t =
  | Initialize(request_t(initialize_params_t))
  | Hover(request_t(hover_params_t))
  | FileOpen(notification_t(file_open_params_t))
  | FileClose(notification_t(file_close_params_t))
  | GoToDefinition(request_t(go_to_definition_params_t));

module Deserialize = {
  open Yojson.Basic.Util;

  let _get_position =
    member("position")
    % (
      fun
      | `Assoc(_) as x => {
          let line = x |> member("line") |> to_int;
          let character = x |> member("character") |> to_int;

          {line, character};
        }
      | x => raise(Type_error("position", x))
    );

  let _get_text_document =
    member("textDocument")
    % (
      fun
      | `Assoc(_) as x => {
          let uri = x |> member("uri") |> to_string;

          {uri: uri};
        }
      | x => raise(Type_error("textDocument", x))
    );

  let request = (get_params, json) => {
    let jsonrpc = json |> member("jsonrpc") |> to_string;
    let id = json |> member("id") |> to_int;
    let method_ = json |> member("method") |> to_string;
    let params = json |> member("params") |> get_params;

    {jsonrpc, id, method: method_, params};
  };

  let notification = (get_params, json) => {
    let jsonrpc = json |> member("jsonrpc") |> to_string;
    let method_ = json |> member("method") |> to_string;
    let params = json |> member("params") |> get_params;

    {jsonrpc, method: method_, params};
  };

  let initialize_req =
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

  let hover_req =
    request(json => {
      let text_document = json |> _get_text_document;
      let position = json |> _get_position;

      {text_document, position};
    });

  let file_open_req =
    notification((json) =>
      (
        {
          let text_document =
            json
            |> member("textDocument")
            |> (
              fun
              | `Assoc(_) as x => {
                  let uri = x |> member("uri") |> to_string;
                  let language_id = x |> member("languageId") |> to_string;
                  let version = x |> member("version") |> to_int;
                  let text = x |> member("text") |> to_string;

                  {uri, language_id, version, text};
                }
              | x => raise(Type_error("textDocument", x))
            );

          {text_document: text_document};
        }: file_open_params_t
      )
    );

  let file_close_req =
    notification((json) =>
      (
        {
          let text_document = json |> _get_text_document;

          {text_document: text_document};
        }: file_close_params_t
      )
    );

  let go_to_definition_req =
    request((json) =>
      (
        {
          let text_document = json |> _get_text_document;
          let position = json |> _get_position;

          {text_document, position, partial_result_token: None};
        }: go_to_definition_params_t
      )
    );
};

let deserialize =
  fun
  | "initialize" => Deserialize.initialize_req % (x => Some(Initialize(x)))
  | "textDocument/hover" => Deserialize.hover_req % (x => Some(Hover(x)))
  | "textDocument/didOpen" =>
    Deserialize.file_open_req % (x => Some(FileOpen(x)))
  | "textDocument/didClose" =>
    Deserialize.file_close_req % (x => Some(FileClose(x)))
  | "textDocument/definition" =>
    Deserialize.go_to_definition_req % (x => Some(GoToDefinition(x)))
  | _ => (_ => None);
