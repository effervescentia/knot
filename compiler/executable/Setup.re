open Kore;

let run = () => {
  let cwd = Unix.getcwd();

  /* configuration */
  Config.set_from_args(cwd);
  let {is_server, is_debug, paths: {config_file}} = Config.get();

  /* logging */
  if (is_debug) {
    Log.set_log_level(Log.DEBUG);
  } else {
    Log.set_log_level(Log.WARN);
  };

  Log.set_output(stdout);
  Log.color_on();

  let pretty_config_path =
    Util.is_within_dir(cwd, config_file) ?
      Util.chop_path_prefix(cwd, config_file) : config_file;

  Log.info("%s  (%s)", Emoji.gear, pretty_config_path);
};
