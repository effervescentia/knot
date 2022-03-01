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
  range: Range.t,
  full_range: Range.t,
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
               Node.Raw.get_value
               % (
                 fun
                 | AST.Declaration(
                     MainExport(name) | NamedExport(name),
                     decl,
                   ) => {
                     let range = Node.Raw.get_range(name);
                     let name = name |> Node.Raw.get_value |> ~@Identifier.pp;
                     let type_ = Node.get_type(decl);

                     Some(
                       switch (Node.get_value(decl)) {
                       | Constant(expr) => {
                           name,
                           detail: type_ |> ~@TypeV2.pp,
                           range,
                           full_range:
                             Range.join(range, Node.get_range(expr)),
                           kind: Capabilities.Variable,
                         }
                       | Function(args, expr) => {
                           name,
                           detail: type_ |> ~@TypeV2.pp,
                           range,
                           full_range:
                             Range.join(range, Node.get_range(expr)),
                           kind: Capabilities.Function,
                         }
                       },
                     );
                   }
                 | _ => None
               ),
             )
      )
      |?: [];

    response(symbols) |> Protocol.reply(req);

  | None => ()
  };
