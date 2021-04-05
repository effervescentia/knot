open Knot.Kore;

open Executable;

/**
 execution entrypoint
 */
let () = {
  Log.init({debug: false, timestamp: false});

  let (config, command) = Config.from_args();

  Print.color := config.color;
  Log.init({debug: config.debug, timestamp: false});

  config.name |> Print.good |> Log.info("project %s");

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
