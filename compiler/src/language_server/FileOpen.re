open Kore;
open Deserialize;
open Yojson.Basic.Util;

type params_t = {text_document: text_document_item_t};

let notification =
  notification(json => {
    let text_document =
      json
      |> member("textDocument")
      |> (
        fun
        | `Assoc(_) as x => {
            let uri = x |> member("uri") |> to_string;
            let language_id = x |> member("languageId") |> to_string;
            let version = x |> member("version") |> to_int;
            let text = x |> member("text") |> to_string;

            {uri, language_id, version, text};
          }
        | x => raise(Type_error("textDocument", x))
      );

    {text_document: text_document};
  });

let handler =
    (
      runtime: Runtime.t,
      {params: {text_document: {uri}}}: notification_t(params_t),
    ) => {
  let path = uri_to_path(uri);

  switch (runtime |> Runtime.resolve(path)) {
  | Some((namespace, {compiler, contexts} as ctx)) =>
    let added = compiler |> Compiler.upsert_module(namespace);

    compiler |> Compiler.incremental(added);
    Runtime.analyze_module(namespace, ctx) |> ignore;

  | None => ()
  };
};
