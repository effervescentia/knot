open Kore;

type t('a) = {
  name: string,
  opts: list(Opt.t),
  resolve: (option(Config.t), global_t) => 'a,
};

let create =
    (
      name: string,
      opts: list(Opt.t),
      resolve: (option(Config.t), global_t) => 'a,
    ) => {
  name,
  opts,
  resolve,
};

let map = (f, {resolve} as cmd: t('a)): t('b) => {
  ...cmd,
  resolve: (static, global) => f(resolve(static, global)),
};

let log_config =
    (global: global_t, name: string, attributes: list((string, string))) => {
  name |> Print.green |> Print.bold |> Log.info("running %s command");
  Log.debug(
    "%s config: %s",
    name,
    [
      ("root_dir", global.root_dir),
      ("color", global.color |> string_of_bool),
      ("debug", global.debug |> string_of_bool),
      ...attributes,
    ]
    |> List.to_seq
    |> Hashtbl.of_seq
    |> Hashtbl.to_string(Functional.identity, Functional.identity)
    |> Pretty.to_string,
  );
};
