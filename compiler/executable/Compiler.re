open Core;
open KnotAnalyze.Core;

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

  global_scope.pending()
  |> List.iter(Debug.print_resolve_target % print_endline);
};

/* let config = {config_file, root_dir, source_dir, build_dir};

   let rec build_compilation_context = global_scope =>
     Linker.link(
       config,
       global_scope,
       Resolver.simple(root_dir, build_compilation_context(global_scope)),
     );

   let () = {
     if (String.sub(in_path, 0, String.length(source_dir)) != source_dir) {
       raise(EntryPointOutsideBuildContext);
     };

     let module_tbl = Hashtbl.create(24);
     let global_scope = Scope.create(~label="global", ~module_tbl, ());

     build_compilation_context(global_scope, in_path);

     Hashtbl.iter(
       key =>
         (
           fun
           | NotLoaded(_) => "NOT LOADED"
           | Loaded(_) => "LOADED"
         )
         % Printf.sprintf("module(%s): %s", key)
         % print_endline,
       global_scope.module_tbl,
     );
   }; */
