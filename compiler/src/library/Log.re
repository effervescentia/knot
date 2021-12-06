/**
 Logging utilities.
 */
open Infix;

module Fmt = Pretty.Formatters;
module ANSI = ANSITerminal;

type config_t = {
  debug: bool,
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

      Dolog.Log.set_prefix_builder(lvl =>
        Fmt.(
          str(
            "%s[knot] %a ",
            cfg.timestamp ? Sys.time() |> string_of_float : "",
            bold(ansi([_color_of_level(lvl)], string)),
            string_of_level(lvl),
          )
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
