open Kore;

let create = (~default=Config.defaults.log_imports, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.log_imports)),
      log_imports_key,
      Unit(() => value := Some(true)),
      "print a graph describing the dependencies between modules",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> Argument.resolve(cfg, x => x.log_imports, default);

  (argument, resolve);
};
