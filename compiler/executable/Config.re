open Kore;

let global = ref(None);

let get = () =>
  switch (global^) {
  | Some(cfg) => cfg
  | None => invariant(ConfigurationNotInitialized)
  };

let relative_path = (extract, absolute_path) => {
  let {paths} = get();
  Util.chop_path_prefix(extract(paths), absolute_path);
};

let is_config_file =
  String.lowercase_ascii
  % (
    fun
    | s when s == Knot.Constants.config_file => true
    | ".knot.yml" => true
    | _ => false
  );

let source_path = relative_path(({source_dir}) => source_dir);
let root_path = relative_path(({root_dir}) => root_dir);
let is_main = path => path == get().main;
let module_name = module_ =>
  is_main(module_) ? Knot.Constants.main_module_alias : module_;

let rec find_file = entry => {
  let dir = Filename.dirname(entry);

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
        if (dir == entry) {
          throw_exec(MissingRootDirectory);
        } else {
          find_file(dir);
        }
    );
  } else {
    find_file(dir);
  };
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

let create_descriptor = (path_resolver, target) => {
  let absolute_path = path_resolver(target);

  KnotCompile.Core.{
    target,
    absolute_path,
    relative_path: source_path(absolute_path),
    pretty_path: module_name(target),
  };
};

let set_from_args = cwd => {
  let module_type = ref(ES6);
  let is_server = ref(false);
  let is_debug = ref(false);
  let config_file = ref("");
  let main = ref("");
  let port = ref(1338);

  Arg.parse(
    [
      ("-debug", Arg.Set(is_debug), " Enable a higher level of logging"),
      (
        "-server",
        Arg.Set(is_server),
        " Run the compiler as a server for incremental or lazily-evaluated builds",
      ),
      (
        "-config",
        Arg.Set_string(config_file),
        " Path to the directory containing your .knot.yml file",
      ),
      (
        "-port",
        Arg.Set_int(port),
        " The port to run on when in server mode",
      ),
      (
        "-compiler.module",
        Arg.Symbol(
          ["common", "es6"],
          choice =>
            module_type :=
              (
                switch (choice) {
                | "common" => Common
                | _ => ES6
                }
              ),
        ),
        "  The module type to generate",
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
    module_type: module_type^,
    is_server: is_server^,
    is_debug: is_debug^,
    port: port^,
    paths:
      find_file(is_server^ ? Util.normalize_path(cwd, config_file^) : main^)
      |> generate_paths,
  };

  if (!config.is_server) {
    if (main^ == "") {
      raise(Arg.Bad("must provide the path to a source file"));
    };

    if (!Util.is_within_dir(config.paths.source_dir, main^)) {
      throw_exec(
        EntryPointOutsideBuildContext(main^, config.paths.source_dir),
      );
    };
  };

  global := Some(config);
};
