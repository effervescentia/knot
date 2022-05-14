open Kore;

let create = (~default=ConfigFile.defaults.color, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.color)),
      color_key,
      Bool(x => value := Some(x)),
      "allow color in logs",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> Argument.resolve(cfg, x => x.color, default);

  (argument, resolve);
};
