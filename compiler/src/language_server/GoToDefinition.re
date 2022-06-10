open Kore;

type params_t = {
  text_document: Protocol.text_document_t,
  position: Protocol.position_t,
  partial_result_token: option(Protocol.progress_token),
};

let method_key = "textDocument/definition";

let deserialize = json => {
  let text_document = Deserialize.text_document(json);
  let position = Deserialize.position(json);

  {text_document, position, partial_result_token: None};
};

let handler: Runtime.request_handler_t(params_t) =
  (runtime, {text_document: {uri}}) => {
    Log.info("go to definition %s", uri);

    Result.ok(`Null);
  };
