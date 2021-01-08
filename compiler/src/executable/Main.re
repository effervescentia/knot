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
  | Static => Static.run({compile: compiler_config})
  | Server =>
    Lwt_main.run @@ Server.run({compile: compiler_config, port: 8080})
  | Watch => Lwt_main.run @@ Watch.run({compile: compiler_config})
  };
};
