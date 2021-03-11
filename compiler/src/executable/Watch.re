/**
 Utilities for the "watch" command.
 */
open Kore;
open Fswatch;

module Watcher = File.Watcher;

type config_t = {
  target: Target.t,
  out_dir: string,
};

let mode = () => {
  let (out_dir_opt, get_out_dir) = Opt.Shared.out_dir();
  let (target_opt, get_target) = Opt.Shared.target();

  let resolve = () => {target: get_target(), out_dir: get_out_dir()};

  ("watch", [out_dir_opt, target_opt], resolve);
};

let run = (cfg: Compiler.config_t, cmd: config_t) => {
  let compiler = Compiler.create(~catch=print_errs % Log.warn("%s"), cfg);

  Sys.set_signal(
    Sys.sigterm,
    Sys.Signal_handle(_ => compiler |> Compiler.teardown),
  );

  compiler |> Compiler.init(~skip_cache=true);

  Log.info("initial compilation successful");

  let watcher = Watcher.create(cfg.source_dir, [Constants.file_extension]);

  watcher
  |> Watcher.(
       watch(actions =>
         actions
         |> List.map(((path, action)) => {
              let id = Internal(path);
              switch (action) {
              | Add => compiler |> Compiler.add_module(id)
              | Update => compiler |> Compiler.update_module(id) |> snd
              | Remove => compiler |> Compiler.remove_module(id)
              | Relocate => compiler |> Compiler.relocate_module(id) |> snd
              };
            })
         |> List.flatten
         |> (updated => compiler |> Compiler.incremental(updated))
       )
     );
};
