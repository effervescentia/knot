/**
 Utilities for the "watch" command.
 */
open Kore;
open Fswatch;

module Watcher = File.Watcher;

type config_t = {
  target: Target.t,
  out_dir: string,
  entry: Namespace.t,
};

let mode = () => {
  let (out_dir_opt, get_out_dir) = Opt.Shared.out_dir();
  let (target_opt, get_target) = Opt.Shared.target();
  let (entry_opt, get_entry) = Opt.Shared.entry();

  Mode.create(
    "watch", [out_dir_opt, target_opt, entry_opt], (static, global) =>
    {
      target: get_target(static),
      out_dir: get_out_dir(static, global.root_dir),
      entry: get_entry(static, global.root_dir, global.source_dir),
    }
  );
};

let run = (global: global_t, config: config_t) => {
  Log.info("running 'watch' command");
  Log.debug(
    "watch config: %s",
    [
      ("name", global.name),
      ("root_dir", global.root_dir),
      ("source_dir", global.source_dir),
      ("out_dir", config.out_dir),
      ("target", config.target |> Target.to_string),
      ("entry", config.entry |> Namespace.to_string),
    ]
    |> List.to_seq
    |> Hashtbl.of_seq
    |> Hashtbl.to_string(Functional.identity, Functional.identity)
    |> Pretty.to_string,
  );

  let compiler =
    Compiler.create(
      ~report=print_errs % Log.warn("%s"),
      {
        name: global.name,
        root_dir: global.root_dir,
        source_dir: global.source_dir,
      },
    );

  Sys.set_signal(
    Sys.sigterm,
    Sys.Signal_handle(_ => compiler |> Compiler.teardown),
  );

  compiler |> Compiler.init(~skip_cache=true, config.entry);

  Log.info("initial compilation successful");

  let watcher =
    Watcher.create(global.source_dir, [Constants.file_extension]);

  watcher
  |> Watcher.(
       watch(actions =>
         actions
         |> List.map(((path, action)) => {
              let id = Namespace.Internal(path);
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
