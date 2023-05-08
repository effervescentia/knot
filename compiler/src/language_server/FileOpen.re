open Kore;

type params_t = {text_document: Protocol.text_document_item_t};

let method_key = "textDocument/didOpen";

let deserialize = json => {
  let text_document = Deserialize.text_document_item(json);

  {text_document: text_document};
};

let handler: Runtime.notification_handler_t(params_t) =
  (runtime, {text_document: {uri, _}, _}) =>
    runtime
    |> Runtime.resolve(uri)
    |> Option.iter(((namespace, Runtime.{compiler, _} as ctx)) => {
         let updated = compiler |> Compiler.upsert_module(namespace);

         compiler |> Compiler.incremental(updated);
         Runtime.analyze_module(namespace, ctx) |> ignore;
       });
