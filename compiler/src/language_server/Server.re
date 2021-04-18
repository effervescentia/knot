open Kore;
open Reference;

module Compiler = Compile.Compiler;
module ScopeTree = Compile.ScopeTree;
module ImportGraph = Resolve.ImportGraph;
module InputStream = File.InputStream;

type module_context_t = {tokens: TokenTree.t};

type compiler_context_t = {
  uri: string,
  compiler: Compiler.t,
  contexts: Hashtbl.t(Namespace.t, module_context_t),
};

let __file_schema = "file://";

let _uri_to_path = String.drop_prefix(__file_schema);

let _reporter = errs =>
  Protocol.send(
    Response.show_message(
      errs
      |> Knot.Error.print_errs
      |> Print.fmt("compilation failed with errors:\n%s"),
      Error,
    ),
  );

let _resolve =
    (path: string, compilers: Hashtbl.t(string, compiler_context_t)) =>
  compilers
  |> Hashtbl.to_seq_values
  |> List.of_seq
  |> List.find_opt(({compiler}) =>
       String.starts_with(Compiler.(compiler.config.root_dir), path)
     )
  |> (
    fun
    | Some({compiler} as context) => {
        let source_dir =
          Filename.concat(
            compiler.config.root_dir,
            compiler.config.source_dir,
          );
        let relative =
          path
          |> Filename.relative_to(source_dir)
          |> String.drop_suffix(Constants.file_extension)
          |> (++)(Constants.root_dir);

        Some((relative |> Namespace.of_string, context));
      }
    | None => {
        Log.warn(
          "event for file %s ignored as it doesn't belong to any active projects",
          path,
        );
        None;
      }
  );

let _force_compile = (namespace: Namespace.t, compiler: Compiler.t) =>
  if (!(compiler.graph |> ImportGraph.has_module(namespace))) {
    let added = compiler |> Compiler.add_module(namespace);

    compiler |> Compiler.incremental(added);
    /* module does not exist in module table */
  } else if (!Hashtbl.mem(compiler.modules, namespace)) {
    compiler |> Compiler.incremental([namespace]);
  };

let _analyze_module =
    (namespace: Namespace.t, {compiler, contexts}: compiler_context_t) =>
  switch (Hashtbl.find_opt(compiler.modules, namespace)) {
  | Some({ast}) =>
    let tokens = ast |> TokenTree.of_ast;

    Hashtbl.add(contexts, namespace, {tokens: tokens});

    Some(tokens);
  | None => None
  };

let _scan_for_token = (point: Cursor.point_t) =>
  InputStream.scan(block =>
    Cursor.is_in_range(block |> Block.cursor |> Cursor.expand, point)
  );

let start = (find_config: string => Config.t) => {
  let compilers: Hashtbl.t(string, compiler_context_t) = Hashtbl.create(1);

  Protocol.subscribe(
    stdin,
    Event.(
      fun
      | Initialize({params: {workspace_folders}}) =>
        /* TODO: handle the case where workspace folders are nested? */
        workspace_folders
        |?: []
        |> List.iter(({uri, name}) => {
             let path = _uri_to_path(uri);
             Log.info("creating compiler for '%s' project (%s)", name, path);

             let config = find_config(path);
             let root_dir =
               (
                 Filename.is_relative(config.root_dir)
                   ? Filename.concat(path, config.root_dir) : config.root_dir
               )
               |> Filename.resolve;

             let compiler =
               Compiler.create(
                 ~report=_ => _reporter,
                 {
                   root_dir,
                   source_dir: config.source_dir,
                   name,
                   fail_fast: false,
                 },
               );

             Hashtbl.add(
               compilers,
               name,
               {uri, compiler, contexts: Hashtbl.create(10)},
             );
           })

      | Hover(
          {params: {text_document: {uri}, position: {line, character}}} as req,
        ) => {
          let path = _uri_to_path(uri);
          let point = Cursor.{line, column: character};

          switch (compilers |> _resolve(path)) {
          | Some((namespace, {compiler, contexts} as ctx)) =>
            let find_token = RangeTree.find_leaf(point);

            (
              switch (Hashtbl.find_opt(contexts, namespace)) {
              | Some({tokens}) => find_token(tokens)
              | None =>
                _force_compile(namespace, compiler);
                _analyze_module(namespace, ctx) |?< find_token;
              }
            )
            |> (
              fun
              | Some((range, Primitive(prim))) =>
                Protocol.reply(
                  req,
                  Response.hover(
                    range,
                    Print.fmt(
                      "`%s`",
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
                  |?< (
                    ({scopes}) => ScopeTree.find_type(id, point, scopes)
                  )
                  |?> (type_ => Type.to_string(type_))
                  |?: "unknown"
                  |> Print.fmt("`%s`")
                  |> Response.hover(range)
                  |> Protocol.reply(req);
                }

              | Some(_) => {
                  Hashtbl.find_opt(compiler.modules, namespace)
                  |?< (({raw}) => raw |> _scan_for_token(point))
                  |?< (
                    block =>
                      switch (block |> Block.value) {
                      | ("import" | "const" | "from" | "main" | "let" | "as") as kwd =>
                        Some((
                          kwd |> Print.fmt("keyword `%s`"),
                          block |> Block.cursor,
                        ))
                      | _ => None
                      }
                  )
                  |> (
                    fun
                    | Some((message, cursor)) =>
                      message
                      |> Response.hover(cursor |> Cursor.expand)
                      |> Protocol.reply(req)
                    | None => Protocol.reply(req, Response.hover_empty)
                  );
                }

              | None =>
                Protocol.reply(
                  req,
                  Response.error(InvalidRequest, "no token found"),
                )
            );

          | None => ()
          };
        }

      | CodeCompletion(
          {params: {text_document: {uri}, position: {line, character}}} as req,
        ) => {
          let path = _uri_to_path(uri);
          let point = Cursor.{line, column: character};

          switch (compilers |> _resolve(path)) {
          | Some((namespace, {compiler})) =>
            Hashtbl.find_opt(compiler.modules, namespace)
            |?< (({scopes}) => ScopeTree.find_scope(point, scopes))
            |?> Hashtbl.to_seq_keys
            % List.of_seq
            % List.map(Export.to_string)
            |?: []
            |> Response.completion
            |> Protocol.reply(req)

          | None => Protocol.reply(req, Response.completion([]))
          };
        }

      | FileOpen({params: {text_document: {uri}}}) => {
          let path = _uri_to_path(uri);

          switch (compilers |> _resolve(path)) {
          | Some((namespace, {compiler, contexts} as ctx)) =>
            let added = compiler |> Compiler.upsert_module(namespace);

            compiler |> Compiler.incremental(added);
            _analyze_module(namespace, ctx) |> ignore;
          | None => ()
          };
        }

      | FileClose({params: {text_document: {uri}}}) => {
          let path = _uri_to_path(uri);

          switch (compilers |> _resolve(path)) {
          | Some((namespace, {compiler, contexts})) =>
            Hashtbl.remove(contexts, namespace)
          | None => ()
          };
        }

      | FileChange({params: {text_document: {uri}, changes}}) => {
          let path = _uri_to_path(uri);

          switch (changes |> List.last, compilers |> _resolve(path)) {
          | (
              Some(contents),
              Some((namespace, {compiler, contexts} as ctx)),
            ) =>
            let silent_compiler = {...compiler, dispatch: ignore};

            silent_compiler |> Compiler.insert_module(namespace, contents);
            _analyze_module(namespace, {...ctx, compiler: silent_compiler})
            |> ignore;

          | _
          | exception _ => ()
          };
        }

      | GoToDefinition(x) =>
        Log.info("go to definition %s", x.params.text_document.uri)
    ),
  );

  Lwt.return();
};
