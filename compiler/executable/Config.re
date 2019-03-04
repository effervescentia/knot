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

let is_config_file =
  String.lowercase_ascii
  % (
    fun
    | ".knot.yml"
    | ".knot.yaml" => true
    | _ => false
  );

let source_path = relative_path(({source_dir}) => source_dir);
let root_path = relative_path(({root_dir}) => root_dir);
let is_main = path => path == get().main;
let module_name = module_ => is_main(module_) ? main_alias : module_;

let rec find_file2 = entry => ();

let rec find_file = entry =>
  if (Sys.file_exists(entry) && Sys.is_directory(entry)) {
    let dir_handle = Unix.opendir(entry);

    let rec find = () =>
      switch (Unix.readdir(dir_handle)) {
      | name =>
        is_config_file(name) ? Some(Filename.concat(entry, name)) : find()
      | exception End_of_file =>
        Unix.closedir(dir_handle);

        None;
      };

    find()
    |> (
      fun
      | Some(res) => res
      | None =>
        if (Filename.dirname(entry) == entry) {
          raise(MissingRootDirectory);
        } else {
          find_file(entry);
        }
    );
  } else {
    find_file(Filename.dirname(entry));
  };

let generate_paths = config_file => {
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
  let is_server = ref(false);
  let config_file = ref("");
  let main = ref("");

  Arg.parse(
    [
      (
        "-server",
        Arg.Set(is_server),
        "run the compiler as a server for incremental or lazily-evaluated builds",
      ),
      (
        "-config",
        Arg.Set_string(config_file),
        "path to the directory containing your .knot.yml file",
      ),
    ],
    x =>
      if (main^ == "") {
        main := Util.normalize_path(cwd, x);
      } else {
        raise(Arg.Bad(Printf.sprintf("unexpected argument: %s", x)));
      },
    "knotc <entrypoint>",
  );

  let config = {
    main: main^,
    is_server: is_server^,
    paths:
      find_file(is_server^ ? Util.normalize_path(cwd, config_file^) : main^)
      |> generate_paths,
  };

  if (!config.is_server) {
    if (main^ == "") {
      raise(Arg.Bad("must provide the path to a source file"));
    };

    if (!Util.is_within_dir(config.paths.source_dir, main^)) {
      raise(EntryPointOutsideBuildContext(main^));
    };
  };

  global := Some(config);
};
