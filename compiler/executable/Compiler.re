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

let () = {
  Printf.printf("root dir: %s\n", root_dir);
  /* normalize_path(in_path) |> Printf.sprintf("loading %s") |> print_endline; */
  let global_scope = Scope.create(~label="global", ());
  let loaded = Loader.load(~global_scope, in_path);

  List.map(Util.real_path(root_dir), loaded.deps)
  |> List.iter(dep =>
       (
         try (Loader.load(~global_scope, dep)) {
         | _ => Printf.sprintf("%sot", dep) |> Loader.load(~global_scope)
         }
       )
       |> ignore
     );

  Generator.generate(print_string, loaded.ast);
};
