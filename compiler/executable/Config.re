open Kore;

exception ConfigurationNotInitialized;

let global = ref(None);

let get = () =>
  switch (global^) {
  | Some(cfg) => cfg
  | None => raise(ConfigurationNotInitialized)
  };

let relative_path = (extract, absolute_path) => {
  let {paths} = get();
  Util.chop_path_prefix(extract(paths), absolute_path);
};
let source_path = relative_path(({source_dir}) => source_dir);
let root_path = relative_path(({root_dir}) => root_dir);
let is_main = path => path == get().main;
let module_name = module_ => is_main(module_) ? main_alias : module_;

let rec find_file = entry => {
  let dir = Filename.dirname(entry);
  let dir_handle = Unix.opendir(dir);

  let rec find = () =>
    switch (Unix.readdir(dir_handle)) {
    | name =>
      switch (String.lowercase_ascii(name)) {
      | ".knot.yml"
      | ".knot.yaml" => Some(Filename.concat(dir, name))
      | _ => find()
      }
    | exception End_of_file =>
      Unix.closedir(dir_handle);

      None;
    };

  find()
  |> (
    fun
    | Some(res) => res
    | None =>
      if (Filename.dirname(dir) == dir) {
        raise(MissingRootDirectory);
      } else {
        find_file(dir);
      }
  );
};

let generate_paths = in_path => {
  let config_file = find_file(in_path);
  let root_dir = Filename.dirname(config_file);

  {
    config_file,
    root_dir,
    source_dir: Filename.concat(root_dir, "src"),
    build_dir: Filename.concat(root_dir, "dist"),
    module_dir: Filename.concat(root_dir, ".knot"),
  };
};

let set_from_args = cwd => {
  let main = ref(None);

  Arg.parse(
    [],
    x =>
      if (main^ == None) {
        main := Some(x);
      } else {
        raise(Arg.Bad(Printf.sprintf("unexpected argument: %s", x)));
      },
    "knotc <entrypoint>",
  );

  let in_path =
    Util.from_option(
      Arg.Bad("must provide the path to a source file"),
      main^,
    )
    |> Util.normalize_path(cwd);
  let config = {main: in_path, paths: generate_paths(in_path)};

  global := Some(config);
};
