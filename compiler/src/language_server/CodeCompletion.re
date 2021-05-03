open Kore;
open Deserialize;
open Yojson.Basic.Util;

type params_t = {
  text_document: text_document_t,
  position: position_t,
  partial_result_token: option(progress_token),
};

type completion_item = {
  label: string,
  kind: Capabilities.symbol_t,
};

let request =
  request(json => {
    let text_document = json |> get_text_document;
    let position = json |> get_position;

    {text_document, position, partial_result_token: None};
  });

let response = (items: list(completion_item)) =>
  `List(
    items
    |> List.map(({label, kind}) =>
         `Assoc([
           ("label", `String(label)),
           ("kind", `Int(kind |> Response.symbol)),
         ])
       ),
  )
  |> Response.wrap;

let handler =
    (
      runtime: Runtime.t,
      {params: {text_document: {uri}, position: {line, character}}} as req:
        request_t(params_t),
    ) => {
  let point = Cursor.{line, column: character};

  switch (runtime |> Runtime.resolve(uri)) {
  | Some((namespace, {compiler})) =>
    Hashtbl.find_opt(compiler.modules, namespace)
    |?< (({scopes}) => ScopeTree.find_scope(point, scopes))
    |?> Hashtbl.to_seq
    % List.of_seq
    % List.map(((key, value)) =>
        {label: key |> Export.to_string, kind: Capabilities.Variable}
      )
    |?: []
    |> response
    |> Protocol.reply(req)

  | None => Protocol.reply(req, response([]))
  };
};
