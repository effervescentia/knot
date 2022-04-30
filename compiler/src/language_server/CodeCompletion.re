open Kore;
open Deserialize;

type params_t = {
  text_document: text_document_t,
  position: position_t,
  partial_result_token: option(progress_token),
};

type completion_item = {
  label: string,
  kind: Capabilities.symbol_t,
};

let method_key = "textDocument/completion";

let deserialize = json => {
  let text_document = get_text_document(json);
  let position = get_position(json);

  {text_document, position, partial_result_token: None};
};

let response = (items: list(completion_item)) =>
  `List(
    items
    |> List.map(({label, kind}) =>
         `Assoc([
           ("label", `String(label)),
           ("kind", `Int(Response.symbol(kind))),
         ])
       ),
  );

let handler: Runtime.request_handler_t(params_t) =
  (runtime: Runtime.t, {text_document: {uri}, position: {line, character}}) => {
    let point = Point.create(line, character);

    switch (runtime |> Runtime.resolve(uri)) {
    | Some((namespace, {compiler})) =>
      Hashtbl.find_opt(compiler.modules, namespace)
      |?< (({scopes}) => ScopeTree.find_scope(point, scopes))
      |?> Hashtbl.to_seq
      % List.of_seq
      % List.map(((key, value)) =>
          {label: key |> ~@Export.pp, kind: Capabilities.Variable}
        )
      |?: []
      |> response
      |> Result.ok

    | None => response([]) |> Result.ok
    };
  };
