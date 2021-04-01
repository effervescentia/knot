/**
 Utilities for the "lsp" command.
 */
open Kore;

type config_t = {port: int};

let mode = () => {
  let (port_opt, get_port) = Opt.Shared.port();

  Mode.create("lsp", [port_opt], (static, global) =>
    {port: get_port(static)}
  );
};

let run = (global: global_t, config: config_t): Lwt.t(unit) => {
  Log.info("running 'lsp' command");
  Log.debug(
    "lsp config: %s",
    [
      ("name", global.name),
      ("root_dir", global.root_dir),
      ("source_dir", global.source_dir),
      ("port", config.port |> string_of_int),
    ]
    |> List.to_seq
    |> Hashtbl.of_seq
    |> Hashtbl.to_string(Functional.identity, Functional.identity)
    |> Pretty.to_string,
  );

  Lwt.return();
};
