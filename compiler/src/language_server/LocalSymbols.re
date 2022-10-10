open Kore;
open ModuleAliases;

type params_t = {
  text_document: Protocol.text_document_t,
  partial_result_token: option(Protocol.progress_token),
};

type document_symbol_t = {
  name: string,
  detail: string,
  kind: Capabilities.symbol_t,
  range: Range.t,
  full_range: Range.t,
};

let method_key = "textDocument/documentSymbol";

let deserialize = json => {
  let text_document = Deserialize.text_document(json);

  {text_document, partial_result_token: None};
};

let response = (symbols: list(document_symbol_t)) =>
  `List(
    symbols
    |> List.map(({name, detail, kind, range, full_range}) =>
         `Assoc([
           ("name", `String(name)),
           ("kind", `Int(Serialize.symbol(kind))),
           ("detail", `String(detail)),
           ("range", Serialize.range(full_range)),
           ("selectionRange", Serialize.range(range)),
         ])
       ),
  );

let handler: Runtime.request_handler_t(params_t) =
  (runtime, {text_document: {uri}}) =>
    switch (runtime |> Runtime.resolve(uri)) {
    | Some((namespace, {compiler})) =>
      let symbols =
        compiler.modules
        |> ModuleTable.find(namespace)
        |?< ModuleTable.(get_entry_data % Option.map(({ast}) => ast))
        |?> List.filter_map(
              fst
              % (
                fun
                | AST.Declaration(
                    MainExport(name) | NamedExport(name),
                    decl,
                  ) => {
                    let range = N.get_range(name);
                    let full_range = N.join_ranges(name, decl);
                    let name = fst(name);
                    let type_ = N.get_type(decl);

                    Some(
                      switch (fst(decl)) {
                      | Constant(expr) => {
                          name,
                          detail: type_ |> ~@Type.pp,
                          range,
                          full_range,
                          kind: Capabilities.Variable,
                        }
                      | Enumerated(variants) => {
                          name,
                          detail: type_ |> ~@Type.pp,
                          range,
                          full_range,
                          kind: Capabilities.Enum,
                        }
                      | Function(args, expr) => {
                          name,
                          detail: type_ |> ~@Type.pp,
                          range,
                          full_range,
                          kind: Capabilities.Function,
                        }
                      | View(props, mixins, expr) => {
                          name,
                          detail: type_ |> ~@Type.pp,
                          range,
                          full_range,
                          kind: Capabilities.Function,
                        }
                      },
                    );
                  }
                | _ => None
              ),
            )
        |?: [];

      symbols |> response |> Result.ok;

    | None => Result.ok(`Null)
    };
