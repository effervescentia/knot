open Kore;

let () = {
  Log.init({debug: false, color: true, timestamp: false});

  let (config, command) = Config.from_args();

  Log.init({debug: config.debug, color: true, timestamp: false});

  Log.info("building project '%s'", config.compile.name);

  switch (command) {
  | Build(cfg) => Build.run(config.compile, cfg)
  | Watch(cfg) => Lwt_main.run @@ Watch.run(config.compile, cfg)
  | Format(cfg) => Format.run(config.compile, cfg)
  | Lint(cfg) => Lint.run(config.compile, cfg)
  | LSP(cfg) => Lwt_main.run @@ LSP.run(config.compile, cfg)
  | Bundle(cfg) => Bundle.run(config.compile, cfg)
  | Develop(cfg) => Lwt_main.run @@ Develop.run(config.compile, cfg)
  };
};
