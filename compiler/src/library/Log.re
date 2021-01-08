let init =
  Dolog.Log.(
    (debug: bool) => {
      set_log_level(debug ? DEBUG : INFO);
      set_output(stdout);
      color_on();
      set_prefix(" [knot]");
    }
  );

let debug = Dolog.Log.debug;
let info = Dolog.Log.info;
let warn = Dolog.Log.warn;
let error = Dolog.Log.error;
let fatal = Dolog.Log.fatal;
