open Kore;
open Yojson.Basic.Util;

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

type text_document_t = {uri: string};

type position_t = {
  line: int,
  character: int,
};

type text_document_item_t = {
  uri: string,
  language_id: string,
  version: int,
  text: string,
};

type progress_token =
  | Int(int)
  | String(string);

let get_position =
  member("position")
  % (
    fun
    | `Assoc(_) as x => {
        let line = (x |> member("line") |> to_int) + 1;
        let character = (x |> member("character") |> to_int) + 1;

        {line, character};
      }
    | x => raise(Type_error("position", x))
  );

let get_uri = member("uri") % to_string % String.drop_prefix(file_schema);

let get_text_document =
  member("textDocument")
  % (
    fun
    | `Assoc(_) as x => {
        let uri = x |> get_uri;

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
