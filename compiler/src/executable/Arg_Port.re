open Kore;

let __min_port = 1024;
let __max_port = 49151;

let _is_in_range = Int.contains((__min_port, __max_port));

let create = (~default=Config.defaults.port, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~alias="p",
      ~default=Int(default),
      ~from_config=cfg => Some(Int(cfg.port)),
      port_key,
      Int(x => value := Some(x)),
      "the port the server runs on",
    );
  let resolve = (cfg: option(Config.t)) => {
    let port = value^ |> Argument.resolve(cfg, x => x.port, default);

    if (!_is_in_range(port)) {
      InvalidArgument(
        port_key,
        Fmt.str("port must be in the range %d-%d", __min_port, __max_port),
      )
      |> fatal;
    };

    port;
  };

  (argument, resolve);
};
