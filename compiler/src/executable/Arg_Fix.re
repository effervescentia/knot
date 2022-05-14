open Kore;

let create = (~default=ConfigFile.defaults.fix, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.fix)),
      fix_key,
      Unit(() => value := Some(true)),
      "automatically apply fixes",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> Argument.resolve(cfg, x => x.fix, default);

  (argument, resolve);
};
