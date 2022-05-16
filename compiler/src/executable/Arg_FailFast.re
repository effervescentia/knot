open Kore;

let create = (~default=Config.defaults.fail_fast, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.fail_fast)),
      fail_fast_key,
      Unit(() => value := Some(true)),
      "fail as soon as the first error is encountered",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> Argument.resolve(cfg, x => x.fail_fast, default);

  (argument, resolve);
};
