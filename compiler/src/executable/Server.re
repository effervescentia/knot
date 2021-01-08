open Kore;

type config_t = {
  compile: Compiler.config_t,
  port: int,
}
and watch_config = {entry: m_id};

let run = (cfg: config_t): Lwt.t(unit) => {
  /* let compiler = Compiler.create(cfg.compile);

     Sys.set_signal(
       Sys.sigterm,
       Sys.Signal_handle(_ => compiler |> Compiler.teardown),
     ); */
  Lwt.return();
};
