open Kore;
open KnotAnalyze.Core;

exception ModuleNotLoaded(string);

let working_dir = Unix.getcwd();
let in_path =
  switch (Sys.argv) {
  | [|_, path|] => Util.normalize_path(working_dir, path)
  | _ => raise(Arg.Bad("must provide the path to a source file"))
  };
let config_file = Util.find_config_file(in_path);
let root_dir = Filename.dirname(config_file);
let source_dir = Filename.concat(root_dir, "src");
let build_dir = Filename.concat(root_dir, "dist");
let module_dir = Filename.concat(root_dir, ".knot");
let config = {config_file, root_dir, source_dir, build_dir, module_dir};

let () = {
  if (String.sub(in_path, 0, String.length(source_dir)) != source_dir) {
    raise(EntryPointOutsideBuildContext(in_path));
  };

  let module_tbl = Hashtbl.create(24);
  let global_scope = Scope.create(~label="global", ~module_tbl, ());
  let path_resolver = PathResolver.simple(config);
  let rec linker = input =>
    Linker.link(path_resolver, global_scope, linker, input);

  try (linker(in_path)) {
  | InvalidPathFormat(s)
  | ModuleDoesNotExist(_, s) when s == in_path =>
    raise(InvalidEntryPoint(in_path))
  };

  if (global_scope.is_resolved()) {
    Printf.sprintf("cleaning build directory: %s", build_dir) |> print_endline;

    Util.clean_directory(build_dir);

    Hashtbl.iter(
      key =>
        fun
        | NotLoaded(_) => raise(ModuleNotLoaded(key))
        | Loaded(s, ast) when Util.is_source_module(key) =>
          Writer.write(config, Util.to_path_segment(key), ast)
        | Loaded(s, ast) when key == in_path => {
            let source_dir_path_length = String.length(source_dir);
            Writer.write(
              config,
              String.sub(
                key,
                source_dir_path_length + 1,
                String.length(key) - source_dir_path_length - 1,
              )
              |> Filename.chop_extension,
              ast,
            );
          }
        | _ => Printf.sprintf("ignoring %s", key) |> print_endline,
      module_tbl,
    );
  } else {
    print_endline(
      "failed to compile, the following statements could not be resolved:",
    );

    global_scope.pending()
    |> List.iter(Debug.print_resolve_target % print_endline);

    exit(-1);
  };
};
