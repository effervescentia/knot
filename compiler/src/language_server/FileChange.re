open Kore;
open Deserialize;
open Yojson.Basic.Util;

type params_t = {
  text_document: text_document_t,
  changes: list(string),
};

let notification =
  notification(json => {
    let text_document = json |> get_text_document;
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
  });

let handler =
    (
      runtime: Runtime.t,
      {params: {text_document: {uri}, changes}}: notification_t(params_t),
    ) =>
  switch (changes |> List.last, runtime |> Runtime.resolve(uri)) {
  | (Some(contents), Some((namespace, {compiler, contexts} as ctx))) =>
    let silent_compiler = {...compiler, dispatch: ignore};

    silent_compiler |> Compiler.insert_module(namespace, contents);
    Runtime.analyze_module(namespace, {...ctx, compiler: silent_compiler})
    |> ignore;

  | _
  | exception _ => ()
  };
