open Knot.Kore;

open Executable;

/**
 execution entrypoint
 */
let () = {
  Log.init({debug: false, color: true, timestamp: false});

  let (config, command) = Config.from_args();

  Log.init({debug: config.debug, color: true, timestamp: false});

  Log.info("project '%s'", config.name);

  switch (command) {
  | Build(cmd) => Build.run(config, cmd)
  | Watch(cmd) => Lwt_main.run @@ Watch.run(config, cmd)
  | Format(cmd) => Format.run(config, cmd)
  | Lint(cmd) => Lint.run(config, cmd)
  | LSP(cmd) => Lwt_main.run @@ LSP.run(config, cmd)
  | Bundle(cmd) => Bundle.run(config, cmd)
  | Develop(cmd) => Lwt_main.run @@ Develop.run(config, cmd)
  };
};
