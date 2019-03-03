open Kore;

let run = () => {
  /* configuration */
  Config.set_from_args(Unix.getcwd());

  let config = Config.get();

  if (Util.is_within_dir(config.paths.source_dir, config.main)) {
    raise(EntryPointOutsideBuildContext(config.main));
  };

  /* logging */
  Log.set_log_level(Log.DEBUG);
  Log.set_output(stdout);
  Log.color_on();
};
