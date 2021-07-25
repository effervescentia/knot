open Kore;

module ImportGraph = Resolve.ImportGraph;

type module_context_t = {tokens: TokenTree.t};

type compiler_context_t = {
  uri: string,
  compiler: Compiler.t,
  contexts: Hashtbl.t(Namespace.t, module_context_t),
};

type t = {
  find_config: string => Config.t,
  compilers: Hashtbl.t(string, compiler_context_t),
};

let resolve = (path: string, {compilers}: t) =>
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

        Some((Namespace.of_string(relative), context));
      }
    | None => {
        Log.warn(
          "event for file %s ignored as it doesn't belong to any active projects",
          path,
        );
        None;
      }
  );

let force_compile = (namespace: Namespace.t, compiler: Compiler.t) =>
  if (!(compiler.graph |> ImportGraph.has_module(namespace))) {
    let added = compiler |> Compiler.add_module(namespace);

    compiler |> Compiler.incremental(added);
    /* module does not exist in module table */
  } else if (!Hashtbl.mem(compiler.modules, namespace)) {
    compiler |> Compiler.incremental([namespace]);
  };

let analyze_module =
    (namespace: Namespace.t, {compiler, contexts}: compiler_context_t) =>
  switch (Hashtbl.find_opt(compiler.modules, namespace)) {
  | Some({ast}) =>
    let tokens = TokenTree.of_ast(ast);

    Hashtbl.add(contexts, namespace, {tokens: tokens});

    Some(tokens);
  | None => None
  };

let scan_for_token = (point: Cursor.point_t) =>
  File.InputStream.scan(block =>
    Cursor.is_in_range(block |> Block.cursor |> Cursor.expand, point)
  );

let purge_module = (path: string, runtime: t) =>
  switch (runtime |> resolve(path)) {
  | Some((namespace, {compiler, contexts})) =>
    Hashtbl.remove(contexts, namespace)
  | None => ()
  };
