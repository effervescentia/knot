open Executable.Kore;

module Processor = Executable.Processor;

let _panic = (err: string) => {
  Log.fatal("%s", err);

  exit(100);
};

let _run = () => {
  Fmt.color := !is_ci_env;

  let (config, command) = Processor.run();

  Log.init({debug: config.debug, timestamp: false});

  Log.info("found project %s", config.name |> ~@Fmt.info_str);

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

/**
 execution entrypoint
 */
let () = {
  Log.init({debug: false, timestamp: false});

  try(_run()) {
  | FatalError(fatal_err) =>
    switch (fatal_err) {
    | MissingCommand => _panic("must provide a command")

    | InvalidArgument(arg_key, message) =>
      Fmt.str("invalid %a option: %s", Fmt.bold_str, arg_key, message)
      |> _panic

    | UnexpectedArgument(arg_key) =>
      Fmt.str("unexpected argument %a", Fmt.bold_str, arg_key) |> _panic

    | UnknownTarget(target) =>
      Fmt.str("unknown target %a", Fmt.bold_str, target) |> _panic

    | InvalidConfigFile(path, message) =>
      Fmt.str("unable to use config file %a: %s", Fmt.bold_str, path, message)
      |> _panic
    }
  };
};
