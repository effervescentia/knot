open Kore;

let serialize = json => {
  let content = json |> Yojson.Basic.to_string;

  Print.fmt(
    "Content-Length: %d\r\n\r\n%s",
    content |> String.length,
    content,
  );
};

let wrap = (result: Yojson.Basic.t, id: int) =>
  `Assoc([
    ("jsonrpc", `String("2.0")),
    ("id", `Int(id)),
    ("result", result),
  ]);

let _wrap_notification = (method_: string, result: Yojson.Basic.t) =>
  `Assoc([
    ("jsonrpc", `String("2.0")),
    ("method", `String(method_)),
    ("result", result),
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

let initialize = (name: string, workspace_support: bool) =>
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
  ])
  |> wrap;

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
  |> _wrap_notification("window/showMessage");
