open Kore;

module ImportGraph = Resolve.ImportGraph;

type module_context_t = {tokens: TokenTree.t};

type compiler_context_t = {
  uri: string,
  compiler: Compiler.t,
  contexts: Hashtbl.t(Namespace.t, module_context_t),
};

type t = {
  server: JSONRPC.Server.t,
  find_config: string => Config.t,
  compilers: Hashtbl.t(string, compiler_context_t),
  stdlib: string,
};

type request_handler_t('a) = JSONRPC.Protocol.Event.request_handler_t(t, 'a);

type notification_handler_t('a) =
  JSONRPC.Protocol.Event.notification_handler_t(t, 'a);

let resolve = (path: string, {compilers, _}: t) =>
  compilers
  |> Hashtbl.to_seq_values
  |> List.of_seq
  |> List.find_opt(({compiler, _}) =>
       String.starts_with(Compiler.(compiler.config.root_dir), path)
     )
  |> (
    fun
    | Some({compiler, _} as context) => {
        let relative =
          path
          |> Filename.relative_to(compiler.config.source_dir)
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
    let updated = compiler |> Compiler.add_module(namespace);

    compiler |> Compiler.incremental(updated);
    /* module does not exist in module table */
  } else if (!ModuleTable.mem(namespace, compiler.modules)) {
    compiler |> Compiler.incremental([namespace]);
  };

let analyze_module =
    (namespace: Namespace.t, {compiler, contexts, _}: compiler_context_t) =>
  compiler.modules
  |> ModuleTable.find(namespace)
  |?< ModuleTable.get_entry_data
  |?> (
    (ModuleTable.{ast, _}) => {
      let tokens = TokenTree.of_ast(ast);

      Hashtbl.add(contexts, namespace, {tokens: tokens});

      tokens;
    }
  );

let scan_for_token = (point: Point.t) =>
  File.InputStream.scan(Node.get_range % Range.contains_point(point));

let purge_module = (path: string, runtime: t) =>
  runtime
  |> resolve(path)
  |> Option.iter(((namespace, {contexts, _})) =>
       Hashtbl.remove(contexts, namespace)
     );
