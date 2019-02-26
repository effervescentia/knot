open Core;

module Generator = KnotGen.Generator;
module Scope = KnotAnalyze.Scope;

let working_dir = Unix.getcwd();
let in_path =
  switch (Sys.argv) {
  | [|_, path|] => Util.normalize_path(working_dir, path)
  | _ => raise(Arg.Bad("must provide the path to a source file"))
  };
let config_file = Util.find_config_file(in_path);
let root_dir = Filename.dirname(config_file);

let rec add_to_execution_context = (module_tbl, path) => {
  let loaded = Loader.load(~module_tbl, path);

  List.map(Util.real_path(root_dir), loaded.deps)
  |> List.iter(dep =>
       (
         try (add_to_execution_context(module_tbl, dep)) {
         | _ =>
           Printf.sprintf("%sot", dep)
           |> add_to_execution_context(module_tbl)
         }
       )
       |> ignore
     );

  loaded;
};

let () = {
  Printf.printf("root dir: %s\n", root_dir);
  /* normalize_path(in_path) |> Printf.sprintf("loading %s") |> print_endline; */
  let module_tbl = Hashtbl.create(64);
  let loaded = add_to_execution_context(module_tbl, in_path);

  Generator.generate(print_string, loaded.ast);
};
