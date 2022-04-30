open Kore;
open Deserialize;

type params_t = {
  text_document: text_document_t,
  position: position_t,
  partial_result_token: option(progress_token),
};

let method_key = "textDocument/definition";

let deserialize = json => {
  let text_document = get_text_document(json);
  let position = get_position(json);

  {text_document, position, partial_result_token: None};
};

let handler: Runtime.request_handler_t(params_t) =
  (runtime, {text_document: {uri}}) => {
    Log.info("go to definition %s", uri);

    Result.ok(`Null);
  };
