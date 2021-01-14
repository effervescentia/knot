open Kore;

type config_t = {
  compile: Compiler.config_t,
  port: int,
};

let run = (cfg: config_t): Lwt.t(unit) => {
  Lwt.return();
};
