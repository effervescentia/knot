open Kore;

let serialize = json => {
  let content = json |> Yojson.Basic.to_string;

  Print.fmt(
    "Content-Length: %d\r\n\r\n%s",
    content |> String.length,
    content,
  );
};

let symbol =
  Capabilities.(
    fun
    | File => 1
    | Module => 2
    | Namespace => 3
    | Package => 4
    | Class => 5
    | Method => 6
    | Property => 7
    | Field => 8
    | Constructor => 9
    | Enum => 10
    | Interface => 11
    | Function => 12
    | Variable => 13
    | Constant => 14
    | String => 15
    | Number => 16
    | Boolean => 17
    | Array => 18
    | Object => 19
    | Key => 20
    | Null => 21
    | EnumMember => 22
    | Struct => 23
    | Event => 24
    | Operator => 25
    | TypeParameter => 26
  );

let range = ((start, end_): Cursor.range_t) =>
  `Assoc([
    (
      "start",
      `Assoc([
        ("line", `Int(start.line - 1)),
        ("character", `Int(start.column - 1)),
      ]),
    ),
    (
      "end",
      `Assoc([
        ("line", `Int(end_.line - 1)),
        ("character", `Int(end_.column)),
      ]),
    ),
  ]);

let wrap = (result: Yojson.Basic.t, id: int) =>
  `Assoc([
    ("jsonrpc", `String("2.0")),
    ("id", `Int(id)),
    ("result", result),
  ]);

let wrap_notification = (method_: string, params: Yojson.Basic.t) =>
  `Assoc([
    ("jsonrpc", `String("2.0")),
    ("method", `String(method_)),
    ("params", params),
  ]);

type error_code_t =
  | ServerNotInitialized
  | UnknownErrorCode
  | ParseError
  | InvalidRequest
  | InvalidParams
  | MethodNotFound
  | InternalError
  | ContentModified
  | RequestCancelled;

let error = (code: error_code_t, message: string, id: int) =>
  `Assoc([
    ("jsonrpc", `String("2.0")),
    ("id", `Int(id)),
    (
      "error",
      `Assoc([
        (
          "code",
          `Int(
            switch (code) {
            | ServerNotInitialized => (-32002)
            | UnknownErrorCode => (-32001)
            | ParseError => (-32700)
            | InvalidRequest => (-32600)
            | InvalidParams => (-32601)
            | MethodNotFound => (-32602)
            | InternalError => (-32603)
            | ContentModified => (-32801)
            | RequestCancelled => (-32800)
            },
          ),
        ),
        ("message", `String(message)),
      ]),
    ),
  ]);

let hover_empty = `Null |> wrap;

type message_t =
  | Error
  | Warning
  | Info
  | Log;

let show_message = (message: string, type_: message_t) =>
  `Assoc([
    ("message", `String(message)),
    (
      "type",
      `Int(
        switch (type_) {
        | Error => 1
        | Warning => 2
        | Info => 3
        | Log => 4
        },
      ),
    ),
  ])
  |> wrap_notification("window/showMessage");
