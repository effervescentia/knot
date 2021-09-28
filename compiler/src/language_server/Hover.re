open Kore;
open Deserialize;
open Yojson.Basic.Util;

type params_t = {
  text_document: text_document_t,
  position: position_t,
};

let request =
  request(json => {
    let text_document = get_text_document(json);
    let position = get_position(json);

    {text_document, position};
  });

let response = (range: Range.t, contents: string) =>
  `Assoc([
    (
      "contents",
      `Assoc([
        ("language", `String("knot")),
        ("value", `String(contents)),
      ]),
    ),
    ("range", Response.range(range)),
  ])
  |> Response.wrap;

let handler =
    (
      runtime: Runtime.t,
      {params: {text_document: {uri}, position: {line, character}}} as req:
        request_t(params_t),
    ) => {
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
        Protocol.reply(
          req,
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
          ),
        )

      | Some((range, Identifier(id))) => {
          Hashtbl.find_opt(compiler.modules, namespace)
          |?< (({scopes}) => ScopeTree.find_type(id, point, scopes))
          |?> ~@Type.pp
          |?> Fmt.str("%a: %s", Identifier.pp, id)
          |?: "(unknown)"
          |> response(range)
          |> Protocol.reply(req);
        }

      | Some(_) => {
          Hashtbl.find_opt(compiler.modules, namespace)
          |?< (({raw}) => raw |> Runtime.scan_for_token(point))
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
              message |> response(range) |> Protocol.reply(req)
            | None => Protocol.reply(req, Response.hover_empty)
          );
        }

      | None =>
        Protocol.reply(req, Response.error(InvalidRequest, "no token found"))
    );

  | None => ()
  };
};
