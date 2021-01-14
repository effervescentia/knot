module ANSI = ANSITerminal;

type config_t = {
  debug: bool,
  color: bool,
  timestamp: bool,
};

let _color_of_level =
  Dolog.Log.(
    fun
    | FATAL => ANSI.red
    | ERROR => ANSI.red
    | WARN => ANSI.yellow
    | INFO => ANSI.cyan
    | DEBUG => ANSI.green
  );

let init =
  Dolog.Log.(
    (cfg: config_t) => {
      set_log_level(cfg.debug ? DEBUG : INFO);
      set_output(stdout);

      if (cfg.color) {
        color_on();
      };

      Dolog.Log.set_prefix_builder(lvl =>
        string_of_level(lvl)
        |> ANSI.sprintf([_color_of_level(lvl)], "%s")
        |> Printf.sprintf(
             "%s[knot] %s ",
             cfg.timestamp ? Sys.time() |> Print.fmt("%f ") : "",
           )
      );
    }
  );

let debug = Dolog.Log.debug;
let info = Dolog.Log.info;
let warn = Dolog.Log.warn;
let error = Dolog.Log.error;
let fatal = Dolog.Log.fatal;
