open Kore;

exception ModuleNotLoaded(string);

let extract_build_tbl = (config, tbl) =>
  Hashtbl.fold(
    (key, value, acc) => {
      switch (value) {
      | NotLoaded(_) => raise(ModuleNotLoaded(key))
      | Loaded(s, ast) when Util.is_source_module(key) =>
        Hashtbl.add(acc, Util.to_path_segment(key), (key, ast))
      | Loaded(s, ast) when key == config.main =>
        Hashtbl.add(
          acc,
          Util.chop_path_prefix(config.paths.source_dir, key)
          |> Filename.chop_extension,
          (main_alias, ast),
        )
      | _ => Log.debug("ignoring %s", key)
      };

      acc;
    },
    tbl,
    Hashtbl.create(16),
  );

let () = {
  Setup.run();

  let {paths} as config = Config.get();
  let module_tbl = Hashtbl.create(24);
  let global_scope = Scope.create(~label="global", ~module_tbl, ());
  let path_resolver = PathResolver.simple(paths);
  let rec linker = input =>
    Linker.link(path_resolver, global_scope, linker, input);

  try (linker(config.main)) {
  | InvalidPathFormat(s)
  | ModuleDoesNotExist(_, s) when s == config.main =>
    raise(InvalidEntryPoint(config.main))
  };

  if (global_scope.is_resolved()) {
    let build_tbl = extract_build_tbl(config, module_tbl);

    Writer.clean_build_dir();
    Hashtbl.iter(Writer.write, build_tbl);
  } else {
    Log.error(
      "%s",
      "failed to compile, the following statements could not be resolved:",
    );

    global_scope.pending()
    |> List.iter(Debug.print_resolve_target % Log.error("%s"));

    exit(-1);
  };
};
