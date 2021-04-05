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

let cmd = () => {
  let (out_dir_opt, get_out_dir) = ConfigOpt.out_dir();
  let (target_opt, get_target) = ConfigOpt.target();
  let (entry_opt, get_entry) = ConfigOpt.entry();

  Cmd.create(
    watch_key, [out_dir_opt, target_opt, entry_opt], (static, global) =>
    {
      target: get_target(static),
      out_dir: get_out_dir(static, global.root_dir),
      entry: get_entry(static, global.root_dir, global.source_dir),
    }
  );
};

let run =
    (
      global: global_t,
      ~report=resolver =>
                Reporter.report(resolver) % File.Writer.write(stderr),
      config: config_t,
    ) => {
  Cmd.log_config(
    global,
    watch_key,
    [
      (out_dir_key, config.out_dir),
      (target_key, config.target |> Target.to_string),
      (entry_key, config.entry |> Namespace.to_string),
    ],
  );

  let compiler =
    Compiler.create(
      ~report,
      {
        name: global.name,
        root_dir: global.root_dir,
        source_dir: global.source_dir,
        fail_fast: false,
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
