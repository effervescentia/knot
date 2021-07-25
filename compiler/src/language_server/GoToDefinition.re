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
    let text_document = get_text_document(json);
    let position = get_position(json);

    {text_document, position, partial_result_token: None};
  });

let handler =
    (
      runtime: Runtime.t,
      {params: {text_document: {uri}}}: request_t(params_t),
    ) => {
  Log.info("go to definition %s", uri);
};
