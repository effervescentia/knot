open Kore;

let () = {
  let config = Config.from_args();

  let root_dir = config.root_dir;
  let name = root_dir |> Filename.basename;
  let entry = Internal(config.entry);
  let compiler_config = Compiler.{name, root_dir, entry};

  Log.init(config.debug);

  Log.info("building project '%s'", name);

  switch (config.mode) {
  | Build => Build.run({compile: compiler_config})
  | Watch => Lwt_main.run @@ Watch.run({compile: compiler_config})
  | Format => Format.run({compile: compiler_config})
  | Lint => Lint.run({compile: compiler_config})
  | LSP => Lwt_main.run @@ LSP.run({compile: compiler_config, port: 8080})
  | Bundle => Bundle.run({compile: compiler_config})
  | Develop =>
    Lwt_main.run @@ Develop.run({compile: compiler_config, port: 8080})
  };
};
