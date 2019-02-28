open Core;

exception EntryPointOutsideBuildContext;

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

let rec add_to_compilation_context = (global_scope, path) => {
  let loaded = Linker.link(global_scope, path);

  List.map(Util.real_path(root_dir), loaded.deps)
  |> List.iter(dep =>
       try (add_to_compilation_context(global_scope, dep)) {
       | _ =>
         Printf.sprintf("%sot", dep)
         |> add_to_compilation_context(global_scope)
       }
     );
};

let () = {
  if (String.sub(in_path, 0, String.length(source_dir)) != source_dir) {
    raise(EntryPointOutsideBuildContext);
  };

  let module_tbl = Hashtbl.create(24);
  let global_scope = Scope.create(~label="global", ~module_tbl, ());

  add_to_compilation_context(global_scope, in_path);
  /* Generator.generate(print_string, loaded.ast); */
};
