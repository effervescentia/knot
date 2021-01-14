open Kore;

type config_t = {compile: Compiler.config_t};

let run = (cfg: config_t) => {
  let compiler =
    Compiler.create(~catch=err => print_err(err) |> panic, cfg.compile);

  compiler |> Compiler.compile;
  compiler |> Compiler.teardown;
};
