open Kore;
open Deserialize;

type params_t = {
  text_document: text_document_t,
  changes: list(string),
};

let method_key = "textDocument/didChange";

let deserialize =
  JSON.Util.(
    json => {
      let text_document = get_text_document(json);
      let changes =
        json
        |> member("contentChanges")
        |> to_list
        |> List.filter_map(
             fun
             | `Assoc([("text", `String(text))]) => Some(text)
             | _ => None,
           );

      {text_document, changes};
    }
  );

let handler: Runtime.notification_handler_t(params_t) =
  (runtime, {text_document: {uri}, changes}) =>
    switch (changes |> List.last, runtime |> Runtime.resolve(uri)) {
    | (Some(contents), Some((namespace, {compiler, contexts} as ctx))) =>
      let silent_compiler = {...compiler, dispatch: ignore};

      silent_compiler |> Compiler.insert_module(namespace, contents);
      Runtime.analyze_module(namespace, {...ctx, compiler: silent_compiler})
      |> ignore;

    | _
    | exception _ => ()
    };
