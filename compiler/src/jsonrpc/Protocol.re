open Kore;

let __content_length_header = "Content-Length";
let __line_break = "\r\n";
let __line_break_chars = String.to_list(__line_break);
let __jsonrpc_key = "jsonrpc";
let __method_key = "method";
let __params_key = "params";
let __id_key = "id";
let __result_key = "result";
let __error_key = "error";
let __code_key = "code";
let __message_key = "message";
let __data_key = "data";
let __version = "2.0";

module Event = {
  type t =
    | Request(int, string, JSON.t)
    | Notification(string, JSON.t);

  type result_t = result(JSON.t, (int, string, option(JSON.t)));

  type request_handler_t('a, 'b) = ('a, 'b) => result_t;
};

module Error = {
  type t =
    | ParseError
    | InvalidRequest
    | MethodNotFound
    | InvalidParams
    | InternalError;

  let to_int =
    fun
    | ParseError => (-32700)
    | InvalidRequest => (-32600)
    | MethodNotFound => (-32601)
    | InvalidParams => (-32602)
    | InternalError => (-32603);

  let get_message =
    fun
    | ParseError => "Invalid JSON was received by the server."
    | InvalidRequest => "The JSON sent is not a valid Request object."
    | MethodNotFound => "The method does not exist / is not available."
    | InvalidParams => "Invalid method parameter(s)."
    | InternalError => "Internal JSON-RPC error.";
};

exception BuiltinError(Error.t);

module Writer = {
  let _serialize = (json: JSON.t): string => {
    let content = JSON.to_string(json);

    Fmt.str(
      "%s: %d\r\n\r\n%s",
      __content_length_header,
      String.length(content),
      content,
    );
  };

  let write_to_channel = (out: out_channel, json: JSON.t): unit =>
    json |> _serialize |> Printf.fprintf(out, "%s");
};

module Reader = {
  let rec _read_headers = stream => {
    let rec loop = buffer => {
      let buffer = buffer ++ (Stream.next(stream) |> String.make(1));

      if (String.ends_with(__line_break, buffer)) {
        buffer |> String.drop_all_suffix(__line_break);
      } else {
        loop(buffer);
      };
    };

    switch (Stream.npeek(2, stream)) {
    | chars when chars == __line_break_chars => []
    | _ =>
      let header = loop("") |> String.split(": ");

      [header, ..._read_headers(stream)];
    };
  };

  let _read_content = (length, stream) => {
    let buffer = Buffer.create(length);

    let rec loop =
      fun
      | 0 => Buffer.contents(buffer)
      | remaining => {
          Buffer.add_char(buffer, Stream.next(stream));
          loop(remaining - 1);
        };

    loop(length);
  };

  let _deserialize =
    JSON.Util.(
      (data: string) => (
        {
          let json =
            try(JSON.from_string(data)) {
            | _ => raise(BuiltinError(ParseError))
            };

          try({
            let id = json |> member(__id_key);
            let method = json |> member(__method_key) |> to_string;
            let params = json |> member(__params_key);

            switch (id, params) {
            | (`Null, `Assoc(_) | `List(_)) => Notification(method, params)
            | (`Int(id), `Assoc(_) | `List(_)) => Request(id, method, params)
            | _ => raise(BuiltinError(InvalidRequest))
            };
          }) {
          | _ => raise(BuiltinError(InvalidRequest))
          };
        }: Event.t
      )
    );

  let read_from_stream = (stream: Stream.t(char)): Event.t => {
    let headers = _read_headers(stream);

    Stream.junk(stream);
    Stream.junk(stream);

    let content_length =
      headers
      |> List.assoc_opt(__content_length_header)
      |> Option.map(int_of_string)
      |?: 0;

    let content = stream |> _read_content(content_length);

    _deserialize(content);
  };
};

/* helpers */

let _id_prop = id => (
  __id_key,
  id |> Option.map(id' => `Int(id')) |?: `Null,
);
let _message = props =>
  `Assoc([(__jsonrpc_key, `String(__version)), ...props]);
let _res_message = (~id=None, props) => _message([_id_prop(id), ...props]);
let _err_message = (~data=None, ~id=None, code: int, message: string) =>
  _res_message(
    ~id,
    [
      (
        __error_key,
        `Assoc(
          [(__code_key, `Int(code)), (__message_key, `String(message))]
          @ (data |> Option.map(data' => [(__data_key, data')]) |?: []),
        ),
      ),
    ],
  );

let response = (id: int, result: JSON.t) =>
  _res_message(~id=Some(id), [(__result_key, result)]);

let error = (~data=None, id: int, code: int, message: string) =>
  _err_message(~data, ~id=Some(id), code, message);

let builtin_error = (~id: option(int)=?, code: Error.t) =>
  _err_message(Error.to_int(code), Error.get_message(code));
