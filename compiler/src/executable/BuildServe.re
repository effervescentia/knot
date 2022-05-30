/**
 Utilities for the "build_serve" command.
 */
open Kore;

module Server = BuildServer.Server;

type config_t = {
  root_dir: string,
  source_dir: string,
  target: Target.t,
  log_imports: bool,
};

let command_key = "build_serve";

let command = () => {
  let (root_dir_arg, get_root_dir) = Arg_RootDir.create();
  let (source_dir_arg, get_source_dir) = Arg_SourceDir.create();
  let (target_arg, get_target) = Arg_Target.create();
  let (log_imports_arg, get_log_imports) = Arg_LogImports.create();

  Command.create(
    command_key,
    [root_dir_arg, source_dir_arg, target_arg, log_imports_arg],
    (static, global) => {
      let root_dir = get_root_dir(static, global.working_dir);
      let source_dir = get_source_dir(static, root_dir).relative;
      let target = get_target(static);
      let log_imports = get_log_imports(static);

      {root_dir, source_dir, target, log_imports};
    },
  );
};

let extract_config = (config: config_t) => [
  (root_dir_key, config.root_dir),
  (source_dir_key, config.source_dir),
  (target_key, config.target |> ~@Target.pp),
  (log_imports_key, string_of_bool(config.log_imports)),
];

let run = (global: Config.global_t, config: config_t) => {
  Util.log_config(global, command_key, extract_config(config));

  Log.info("%s", "starting build server" |> ~@Fmt.warn_str);

  Server.start({
    name: global.name,
    root_dir: config.root_dir,
    source_dir: config.source_dir,
    target: config.target,
    log_imports: config.log_imports,
    stdlib: global.stdlib,
  });
};