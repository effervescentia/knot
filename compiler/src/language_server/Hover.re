open Kore;
open Deserialize;
open Yojson.Basic.Util;

type params_t = {
  text_document: text_document_t,
  position: position_t,
};

let request =
  request(json => {
    let text_document = json |> get_text_document;
    let position = json |> get_position;

    {text_document, position};
  });

let response = (range: Cursor.range_t, contents: string) =>
  `Assoc([
    (
      "contents",
      `Assoc([
        ("language", `String("knot")),
        ("value", `String(contents)),
      ]),
    ),
    ("range", range |> Response.range),
  ])
  |> Response.wrap;

let handler =
    (
      runtime: Runtime.t,
      {params: {text_document: {uri}, position: {line, character}}} as req:
        request_t(params_t),
    ) => {
  let point = Cursor.{line, column: character};

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
            Print.fmt(
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
          |?> (type_ => Type.to_string(type_))
          |?> Print.fmt("%s: %s", id |> Identifier.to_string)
          |?: "(unknown)"
          |> response(range)
          |> Protocol.reply(req);
        }

      | Some(_) => {
          Hashtbl.find_opt(compiler.modules, namespace)
          |?< (({raw}) => raw |> Runtime.scan_for_token(point))
          |?< (
            block =>
              switch (block |> Block.value) {
              | ("import" | "const" | "from" | "main" | "let" | "as") as kwd =>
                Some((
                  kwd |> Print.fmt("(keyword) %s"),
                  block |> Block.cursor,
                ))
              | _ => None
              }
          )
          |> (
            fun
            | Some((message, cursor)) =>
              message
              |> response(cursor |> Cursor.expand)
              |> Protocol.reply(req)
            | None => Protocol.reply(req, Response.hover_empty)
          );
        }

      | None =>
        Protocol.reply(req, Response.error(InvalidRequest, "no token found"))
    );

  | None => ()
  };
};
