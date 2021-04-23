open Kore;
open Deserialize;
open Yojson.Basic.Util;

type params_t = {
  text_document: text_document_t,
  partial_result_token: option(progress_token),
};

type document_symbol_t = {
  name: string,
  detail: string,
  kind: Capabilities.symbol_t,
  range: Cursor.range_t,
  full_range: Cursor.range_t,
};

let request =
  request(json => {
    let text_document = json |> get_text_document;

    {text_document, partial_result_token: None};
  });

let response = (symbols: list(document_symbol_t)) =>
  `List(
    symbols
    |> List.map(({name, detail, kind, range, full_range}) =>
         `Assoc([
           ("name", `String(name)),
           ("kind", `Int(kind |> Response.symbol)),
           ("detail", `String(detail)),
           ("range", full_range |> Response.range),
           ("selectionRange", range |> Response.range),
         ])
       ),
  )
  |> Response.wrap;

let handler =
    (
      runtime: Runtime.t,
      {params: {text_document: {uri}}} as req: request_t(params_t),
    ) =>
  switch (runtime |> Runtime.resolve(uri)) {
  | Some((namespace, {compiler})) =>
    let symbols =
      Hashtbl.find_opt(compiler.modules, namespace)
      |?> (
        ({ast}) =>
          ast
          |> List.filter_map(
               AST.(
                 fun
                 | Declaration(
                     MainExport(name) | NamedExport(name),
                     Constant(expr),
                   ) =>
                   Some({
                     name: name |> Block.value |> Identifier.to_string,
                     detail: expr |> Tuple.snd3 |> Type.to_string,
                     range: name |> Block.cursor |> Cursor.expand,
                     full_range:
                       Cursor.join(name |> Block.cursor, expr |> Tuple.thd3)
                       |> Cursor.expand,
                     kind: Capabilities.Variable,
                   })
                 | Import(_) => None
               ),
             )
      )
      |?: [];

    response(symbols) |> Protocol.reply(req);

  | None => ()
  };
