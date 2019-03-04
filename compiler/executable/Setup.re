open Kore;

let run = () => {
  let cwd = Unix.getcwd();

  /* configuration */
  Config.set_from_args(cwd);

  /* logging */
  Log.set_log_level(Log.DEBUG);
  Log.set_output(stdout);
  Log.color_on();

  let {is_server, paths: {config_file}} = Config.get();
  let pretty_config_file =
    Util.is_within_dir(cwd, config_file) ?
      Util.chop_path_prefix(cwd, config_file) : config_file;

  Log.info("%s  (%s)", Emoji.gear, pretty_config_file);
};
