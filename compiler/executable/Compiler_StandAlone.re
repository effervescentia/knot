open Kore;
open Compiler_Util;

let run = ({paths, main as in_path} as config) => {
  let module_tbl = Hashtbl.create(24);
  let global_scope = Scope.create(~label="global", ~module_tbl, ());
  let path_resolver = PathResolver.simple(paths);
  let rec linker = input =>
    Linker.link(path_resolver, global_scope, linker, input);

  try (linker(in_path)) {
  | InvalidPathFormat(s)
  | ModuleDoesNotExist(_, s) when s == in_path =>
    raise(InvalidEntryPoint(in_path))
  };

  if (global_scope.is_resolved()) {
    let build_tbl = extract_build_tbl(config, module_tbl);

    Writer.clean_build_dir();
    Hashtbl.iter(Writer.write, build_tbl);

    Log.info("%s  done!", Emoji.confetti_ball);
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
