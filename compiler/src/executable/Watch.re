open Kore;
open Fswatch;

module Watcher = File.Watcher;

type config_t = {compile: Compiler.config_t};

let run = (cfg: config_t) => {
  let compiler =
    Compiler.create(~catch=print_err % Log.warn("%s"), cfg.compile);

  Sys.set_signal(
    Sys.sigterm,
    Sys.Signal_handle(_ => compiler |> Compiler.teardown),
  );

  compiler |> Compiler.initialize(~cache=false);

  Log.info("initial compilation successful");

  let watcher =
    Watcher.create(cfg.compile.root_dir, [Constants.file_extension]);

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
