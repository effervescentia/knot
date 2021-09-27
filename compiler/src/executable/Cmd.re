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
  Log.info("running %a command", ~$Fmt.bold(Fmt.green(Fmt.string)), name);
  Log.debug(
    "%s config: %a",
    name,
    ~$Hashtbl.pp(Fmt.string, Fmt.string),
    [
      ("root_dir", global.root_dir),
      ("color", string_of_bool(global.color)),
      ("debug", string_of_bool(global.debug)),
      ...attributes,
    ]
    |> List.to_seq
    |> Hashtbl.of_seq,
  );
};
