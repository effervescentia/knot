open Kore;

type params_t = {
  text_document: Protocol.text_document_t,
  position: Protocol.position_t,
};

let method_key = "textDocument/hover";

let deserialize =
  JSON.Util.(
    json => {
      let text_document = Deserialize.text_document(json);
      let position = Deserialize.position(json);

      {text_document, position};
    }
  );

let response = (range: Range.t, contents: string) =>
  `Assoc([
    (
      "contents",
      `Assoc([
        ("language", `String(Target.knot)),
        ("value", `String(contents)),
      ]),
    ),
    ("range", Serialize.range(range)),
  ]);

let handler: Runtime.request_handler_t(params_t) =
  (runtime, {text_document: {uri}, position: {line, character}}) => {
    let point = Point.create(line, character);

    switch (runtime |> Runtime.resolve(uri)) {
    | Some((namespace, {compiler, contexts} as ctx)) =>
      let find_token = RangeTree.find_leaf(point);

      (
        switch (Hashtbl.find_opt(contexts, namespace)) {
        | Some({tokens}) => find_token(tokens)
        | None =>
          Runtime.force_compile(namespace, compiler);
          Runtime.analyze_module(namespace, ctx) |?< find_token;
        }
      )
      |> (
        fun
        | Some((range, Primitive(prim))) =>
          response(
            range,
            Fmt.str(
              "type %s",
              switch (prim) {
              | Nil => "nil"
              | Boolean(_) => "bool"
              | String(_) => "string"
              | Number(Float(_)) => "float"
              | Number(Integer(_)) => "int"
              },
            ),
          )
          |> Result.ok

        | Some((range, Identifier(id))) => {
            Hashtbl.find_opt(compiler.modules, namespace)
            |?< ModuleTable.(
                  get_entry_data % Option.map(({scopes}) => scopes)
                )
            |?< ScopeTree.find_type(id, point)
            |?> ~@Type.pp
            |?> Fmt.str("%a: %s", Identifier.pp, id)
            |?: "(unknown)"
            |> response(range)
            |> Result.ok;
          }

        | Some(_) => {
            Hashtbl.find_opt(compiler.modules, namespace)
            |?< ModuleTable.(get_entry_data % Option.map(({raw}) => raw))
            |?< Runtime.scan_for_token(point)
            |?< (
              node =>
                switch (Node.Raw.get_value(node)) {
                | ("import" | "const" | "from" | "main" | "let" | "as") as kwd =>
                  Some((
                    Fmt.str("(keyword) %s", kwd),
                    Node.Raw.get_range(node),
                  ))
                | _ => None
                }
            )
            |> (
              fun
              | Some((message, range)) =>
                message |> response(range) |> Result.ok
              | None => Result.ok(`Null)
            );
          }

        | None => Result.ok(`Null)
      );

    | None => Result.ok(`Null)
    };
  };
