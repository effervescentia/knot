open Kore;

module ModuleTable = AST.ModuleTable;

type params_t = {
  text_document: Protocol.text_document_t,
  position: Protocol.position_t,
  partial_result_token: option(Protocol.progress_token),
};

type completion_item = {
  label: string,
  kind: Capabilities.symbol_t,
};

let method_key = "textDocument/completion";

let deserialize = json => {
  let text_document = Deserialize.text_document(json);
  let position = Deserialize.position(json);

  {text_document, position, partial_result_token: None};
};

let response = (items: list(completion_item)) =>
  `List(
    items
    |> List.map(({label, kind}) =>
         `Assoc([
           ("label", `String(label)),
           ("kind", `Int(Serialize.symbol(kind))),
         ])
       ),
  );

let handler: Runtime.request_handler_t(params_t) =
  (runtime, {text_document: {uri, _}, position: {line, character, _}, _}) => {
    let point = Point.create(line, character);

    switch (runtime |> Runtime.resolve(uri)) {
    | Some((namespace, {compiler, _})) =>
      compiler.modules
      |> ModuleTable.find(namespace)
      |?< ModuleTable.(get_entry_data % Option.map(({scopes, _}) => scopes))
      |?< ScopeTree.find_scope(point)
      |?> List.map(((key, _)) =>
            {label: key |> ~@Export.pp, kind: Capabilities.Variable}
          )
      |?: []
      |> response
      |> Result.ok

    | None => response([]) |> Result.ok
    };
  };
