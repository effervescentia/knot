open Globals;

let is_config_file =
  String.lowercase_ascii
  % (
    fun
    | s when s == Knot.Constants.config_file => true
    | ".knot.yml" => true
    | _ => false
  );

let module_name = (config, module_) =>
  module_ == config.main ? Knot.Constants.main_module_alias : module_;

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

let create_descriptor = (config, path_resolver, target) => {
  let absolute_path = path_resolver(target);

  KnotCompile.Globals.{
    target,
    absolute_path,
    relative_path:
      FileUtil.relative_path(config.paths.source_dir, absolute_path),
    pretty_path: module_name(config, target),
  };
};

let from_args = cwd => {
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
        Printf.sprintf(" The port to run on when in server mode (%d)", port^),
      ),
      (
        "-compiler.module",
        Arg.Symbol(
          [common_module, es6_module],
          choice =>
            module_type :=
              (
                switch (choice) {
                | s when s == common_module => Common
                | _ => ES6
                }
              ),
        ),
        Printf.sprintf(
          "  The module type to generate (%s)",
          switch (module_type^) {
          | ES6 => es6_module
          | Common => common_module
          },
        ),
      ),
    ],
    x =>
      if (main^ == "") {
        main := FileUtil.normalize_path(cwd, x);
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
      find_file(
        is_server^ ? FileUtil.normalize_path(cwd, config_file^) : main^,
      )
      |> generate_paths,
  };

  if (!config.is_server) {
    if (main^ == "") {
      raise(Arg.Bad("must provide the path to a source file"));
    };

    if (!FileUtil.is_within_dir(config.paths.source_dir, main^)) {
      throw_exec(
        EntryPointOutsideBuildContext(main^, config.paths.source_dir),
      );
    };
  };

  config;
};
