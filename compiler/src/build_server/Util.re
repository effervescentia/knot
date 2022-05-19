open Kore;

module Status = Runtime.Status;

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

let process_incremental =
    (f: unit => (list(Namespace.t), list(Namespace.t)), runtime: Runtime.t) => {
  Log.info(
    "status %s -> %s",
    Status.to_string(Idle) |> ~@Fmt.grey_str,
    Status.to_string(Running) |> ~@Fmt.bold_str,
  );

  runtime.status = Running;

  let (removed, updated) = f();

  if (!List.is_empty(removed)) {
    Log.debug("%s", ("removed module(s)", removed) |> ~@_pp_module_list);

    Log.info(
      "removed %s module(s)",
      removed |> List.length |> ~@Fmt.(info(int)),
    );
  };

  if (!List.is_empty(updated)) {
    Log.debug("%s", ("processing module(s)", updated) |> ~@_pp_module_list);

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
};
