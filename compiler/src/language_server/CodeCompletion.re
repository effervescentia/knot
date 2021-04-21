open Kore;
open Deserialize;
open Yojson.Basic.Util;

type params_t = {
  text_document: text_document_t,
  position: position_t,
  partial_result_token: option(progress_token),
};

let request =
  request(json => {
    let text_document = json |> get_text_document;
    let position = json |> get_position;

    {text_document, position, partial_result_token: None};
  });

let response = (items: list(string)) =>
  `List(items |> List.map(label => `Assoc([("label", `String(label))])))
  |> Response.wrap;

let handler =
    (
      runtime: Runtime.t,
      {params: {text_document: {uri}, position: {line, character}}} as req:
        request_t(params_t),
    ) => {
  let path = uri_to_path(uri);
  let point = Cursor.{line, column: character};

  switch (runtime |> Runtime.resolve(path)) {
  | Some((namespace, {compiler})) =>
    Hashtbl.find_opt(compiler.modules, namespace)
    |?< (({scopes}) => ScopeTree.find_scope(point, scopes))
    |?> Hashtbl.to_seq_keys
    % List.of_seq
    % List.map(Export.to_string)
    |?: []
    |> response
    |> Protocol.reply(req)

  | None => Protocol.reply(req, response([]))
  };
};
