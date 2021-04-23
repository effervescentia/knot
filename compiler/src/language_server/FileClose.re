open Kore;
open Deserialize;
open Yojson.Basic.Util;

type params_t = {text_document: text_document_t};

let notification =
  notification(json => {
    let text_document = json |> get_text_document;

    {text_document: text_document};
  });

let handler =
    (
      runtime: Runtime.t,
      {params: {text_document: {uri}}}: notification_t(params_t),
    ) =>
  runtime |> Runtime.purge_module(uri);
