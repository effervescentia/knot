open Knot.Kore;

open Executable;

/**
 execution entrypoint
 */
let () = {
  Log.init({debug: false, timestamp: false});

  let (config, command) = Args.to_config();

  Fmt.color := config.color;
  Log.init({debug: config.debug, timestamp: false});

  Log.info("project %a", ~$Fmt.good_str, config.name);

  switch (command) {
  | Build(cmd) => Build.run(config, cmd)
  | Watch(cmd) => Lwt_main.run @@ Watch.run(config, cmd)
  | Format(cmd) => Format.run(config, cmd)
  | Lint(cmd) => Lint.run(config, cmd)
  | Bundle(cmd) => Bundle.run(config, cmd)
  | DevServe(cmd) => Lwt_main.run @@ DevServe.run(config, cmd)
  | LangServe(cmd) => Lwt_main.run @@ LangServe.run(config, cmd)
  | BuildServe(cmd) => Lwt_main.run @@ BuildServe.run(config, cmd)
  };
};
