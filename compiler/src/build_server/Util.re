open Kore;

module Status = Runtime.Status;

type namespace_error_t =
  | NotInSource
  | NotAbsolute;

let _pp_module_list =
  Fmt.(
    root((ppf, (header, modules)) =>
      pf(
        ppf,
        "%s%a",
        header,
        indented(
          list(~layout=Vertical, ~sep=Sep.newline, ppf =>
            pf(ppf, "• %a", bold(Namespace.pp))
          ),
        ),
        modules,
      )
    )
  );

let resolve_namespace =
    (path: string, {compiler: {config}}: Runtime.t)
    : result(Namespace.t, namespace_error_t) => {
  let source_dir = Filename.concat(config.root_dir, config.source_dir);

  if (Filename.is_relative(path)) {
    Error(NotAbsolute);
  } else if (path |> String.starts_with(source_dir)) {
    path |> Filename.relative_to(source_dir) |> Namespace.of_path |> Result.ok;
  } else {
    Error(NotInSource);
  };
};

let process_incremental =
    (
      path: string,
      import: Namespace.t => (list(Namespace.t), list(Namespace.t)),
      runtime: Runtime.t,
    ) => {
  switch (runtime |> resolve_namespace(path)) {
  | Ok(namespace) =>
    Log.info(
      "status %s -> %s",
      Status.to_string(Idle) |> ~@Fmt.grey_str,
      Status.to_string(Running) |> ~@Fmt.bold_str,
    );

    runtime.status = Running;

    let (removed, updated) = import(namespace);

    if (!List.is_empty(removed)) {
      Log.debug("%s", ("removed module(s)", removed) |> ~@_pp_module_list);

      Log.info(
        "removed %s module(s)",
        removed |> List.length |> ~@Fmt.(info(int)),
      );
    };

    if (!List.is_empty(updated)) {
      Log.debug(
        "%s",
        ("processing module(s)", updated) |> ~@_pp_module_list,
      );

      Log.info(
        "compiling %s module(s)",
        updated |> List.length |> ~@Fmt.(info(int)),
      );
    };

    runtime.compiler |> Compiler.incremental(updated);

    if (runtime.log_imports) {
      Log.info(
        "imports:\n%s",
        runtime.compiler.graph |> ~@Resolve.ImportGraph.pp,
      );
    };

    Log.info(
      "status %s -> %s",
      Status.to_string(Running) |> ~@Fmt.grey_str,
      Status.to_string(Idle) |> ~@Fmt.bold_str,
    );

    runtime.status = Idle;

  | Error(NotInSource) =>
    Log.warn(
      "could not find module %s within source directory %s",
      path |> ~@Fmt.bold_str,
      runtime.compiler.config.source_dir |> ~@Fmt.bold_str,
    )

  | Error(NotAbsolute) =>
    Log.warn("rejected relative path module %s", path |> ~@Fmt.bold_str)
  };
};
