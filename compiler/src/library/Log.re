/**
 Logging utilities.
 */
include Infix;

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

/* static */

let init =
  Dolog.Log.(
    (cfg: config_t) => {
      set_log_level(cfg.debug ? DEBUG : INFO);
      set_output(stderr);

      if (cfg.color) {
        color_on();
      };

      Dolog.Log.set_prefix_builder(lvl =>
        string_of_level(lvl)
        |> (
          cfg.color
            ? ANSI.sprintf([_color_of_level(lvl)], "%s")
            : Functional.identity
        )
        |> Print.bold
        |> Print.fmt(
             "%s[knot] %s",
             cfg.timestamp ? Sys.time() |> string_of_float : "",
           )
      );
    }
  );

/* methods */

let debug = Dolog.Log.debug;
let info = Dolog.Log.info;
let warn = Dolog.Log.warn;
let error = Dolog.Log.error;
let fatal = Dolog.Log.fatal;
