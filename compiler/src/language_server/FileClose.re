open Kore;

type params_t = {text_document: Protocol.text_document_t};

let method_key = "textDocument/didClose";

let deserialize = json => {
  let text_document = Deserialize.text_document(json);

  {text_document: text_document};
};

let handler: Runtime.notification_handler_t(params_t) =
  (runtime, {text_document: {uri}}) =>
    runtime |> Runtime.purge_module(uri);
