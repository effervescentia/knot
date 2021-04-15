open Kore;

let serialize = json => {
  let content = json |> Yojson.Basic.to_string;

  Print.fmt(
    "Content-Length: %d\r\n\r\n%s",
    content |> String.length,
    content,
  );
};

let _wrap_response = (result, id: int) =>
  `Assoc([
    ("jsonrpc", `String("2.0")),
    ("id", `Int(id)),
    ("result", result),
  ]);

let _wrap_notification = (method_: string, result) =>
  `Assoc([
    ("jsonrpc", `String("2.0")),
    ("method", `String(method_)),
    ("result", result),
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
        ("completionProvider", `Assoc([("resolveProvider", `Bool(true))])),
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
  |> _wrap_response;

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
