open Kore;

/**
 execution entrypoint
 */
let () = {
  Log.init({debug: false, timestamp: false});

  let (config, command) = Processor.run();

  Fmt.color := config.color;
  Log.init({debug: config.debug, timestamp: false});

  Log.info("project %a", ~$Fmt.good_str, config.name);

  switch (command) {
  | Build(cmd) => Executable.Build.run(config, cmd)
  | Watch(cmd) => Lwt_main.run @@ Executable.Watch.run(config, cmd)
  | Format(cmd) => Executable.Format.run(config, cmd)
  | Lint(cmd) => Executable.Lint.run(config, cmd)
  | Bundle(cmd) => Executable.Bundle.run(config, cmd)
  | DevServe(cmd) => Lwt_main.run @@ Executable.DevServe.run(config, cmd)
  | LangServe(cmd) => Lwt_main.run @@ Executable.LangServe.run(config, cmd)
  | BuildServe(cmd) => Lwt_main.run @@ Executable.BuildServe.run(config, cmd)
  };
};
