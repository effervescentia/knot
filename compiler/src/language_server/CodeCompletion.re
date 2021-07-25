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
    let text_document = get_text_document(json);
    let position = get_position(json);

    {text_document, position, partial_result_token: None};
  });

let response = (items: list(completion_item)) =>
  `List(
    items
    |> List.map(({label, kind}) =>
         `Assoc([
           ("label", `String(label)),
           ("kind", `Int(Response.symbol(kind))),
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
        {label: Export.to_string(key), kind: Capabilities.Variable}
      )
    |?: []
    |> response
    |> Protocol.reply(req)

  | None => Protocol.reply(req, response([]))
  };
};
