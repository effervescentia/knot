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
