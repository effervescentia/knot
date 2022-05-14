open Kore;

let create = (~default=ConfigFile.defaults.debug, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.debug)),
      debug_key,
      Unit(() => value := Some(true)),
      "enable a higher level of logging",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> Argument.resolve(cfg, x => x.debug, default);

  (argument, resolve);
};
