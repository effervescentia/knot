open Kore;

type params_t = {text_document: Protocol.text_document_item_t};

let method_key = "textDocument/didOpen";

let deserialize =
  JSON.Util.(
    json => {
      let text_document = Deserialize.text_document_item(json);

      {text_document: text_document};
    }
  );

let handler: Runtime.notification_handler_t(params_t) =
  (runtime, {text_document: {uri}}) =>
    switch (runtime |> Runtime.resolve(uri)) {
    | Some((namespace, {compiler, contexts} as ctx)) =>
      let added = compiler |> Compiler.upsert_module(namespace);

      compiler |> Compiler.incremental(added);
      Runtime.analyze_module(namespace, ctx) |> ignore;

    | None => ()
    };
