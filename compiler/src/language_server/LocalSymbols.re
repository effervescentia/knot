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
    let text_document = get_text_document(json);

    {text_document, partial_result_token: None};
  });

let response = (symbols: list(document_symbol_t)) =>
  `List(
    symbols
    |> List.map(({name, detail, kind, range, full_range}) =>
         `Assoc([
           ("name", `String(name)),
           ("kind", `Int(Response.symbol(kind))),
           ("detail", `String(detail)),
           ("range", Response.range(full_range)),
           ("selectionRange", Response.range(range)),
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
                 | Declaration(MainExport(name) | NamedExport(name), decl) => {
                     let name_cursor = Node.Raw.cursor(name);
                     let range = Cursor.expand(name_cursor);
                     let name = name |> Node.Raw.value |> Identifier.to_string;
                     let type_ = decl |> Grammar.TypeOf.declaration;

                     Some(
                       switch (decl) {
                       | Constant(expr) => {
                           name,
                           detail: Type.to_string(type_),
                           range,
                           full_range:
                             Cursor.join(name_cursor, Node.cursor(expr))
                             |> Cursor.expand,
                           kind: Capabilities.Variable,
                         }
                       | Function(args, expr) => {
                           name,
                           detail: Type.to_string(type_),
                           range,
                           full_range:
                             Cursor.join(name_cursor, Node.cursor(expr))
                             |> Cursor.expand,
                           kind: Capabilities.Function,
                         }
                       },
                     );
                   }
                 | Import(_) => None
               ),
             )
      )
      |?: [];

    response(symbols) |> Protocol.reply(req);

  | None => ()
  };
