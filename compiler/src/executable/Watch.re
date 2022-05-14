/**
 Utilities for the "watch" command.
 */
open Kore;
open Fswatch;
open File.FilesystemDriver;

module Watcher = File.Watcher;

type config_t = {
  target: Target.t,
  root_dir: string,
  source_dir: string,
  out_dir: string,
  entry: Namespace.t,
};

let _is_source_file = String.ends_with(Constants.file_extension);

let command_key = "watch";

let command = () => {
  let (root_dir_arg, get_root_dir) = Arg_RootDir.create();
  let (source_dir_arg, get_source_dir) = Arg_SourceDir.create();
  let (out_dir_arg, get_out_dir) = Arg_OutDir.create();
  let (entry_arg, get_entry) = Arg_Entry.create();
  let (target_arg, get_target) = Arg_Target.create();

  Command.create(
    command_key,
    [root_dir_arg, source_dir_arg, out_dir_arg, target_arg, entry_arg],
    (static, global) => {
      let root_dir = get_root_dir(static, global.working_dir);
      let source_dir = get_source_dir(static, root_dir);
      let out_dir = get_out_dir(static, root_dir);
      let entry = get_entry(static, source_dir.absolute);
      let target = get_target(static);

      {root_dir, source_dir: source_dir.relative, out_dir, entry, target};
    },
  );
};

let run =
    (
      global: Config.global_t,
      ~report=resolver =>
                Reporter.report(resolver) % File.Writer.write(stderr),
      config: config_t,
    ) => {
  Util.log_config(
    global,
    command_key,
    [
      (root_dir_key, config.root_dir),
      (source_dir_key, config.source_dir),
      (out_dir_key, config.out_dir),
      (entry_key, config.entry |> ~@Namespace.pp),
      (target_key, config.target |> ~@Target.pp),
    ],
  );

  let compiler =
    Compiler.create(
      ~report,
      {
        name: global.name,
        root_dir: config.root_dir,
        source_dir: config.source_dir,
        fail_fast: false,
        log_imports: false,
      },
    );

  Sys.set_signal(
    Sys.sigterm,
    Sys.Signal_handle(_ => Compiler.teardown(compiler)),
  );

  compiler |> Compiler.init(~skip_cache=true, config.entry);

  Log.info("initial compilation successful");

  let watcher = Watcher.create(config.source_dir);

  watcher
  |> Watcher.(
       watch(actions =>
         actions
         |> List.map(((path, action)) => {
              let id = Namespace.Internal(path);
              switch (action) {
              | Add when _is_source_file(path) =>
                compiler |> Compiler.upsert_module(id)
              | Update when _is_source_file(path) =>
                compiler |> Compiler.update_module(id) |> snd
              | Remove when _is_source_file(path) =>
                compiler |> Compiler.remove_module(id)

              | _ => []
              };
            })
         |> List.flatten
         |> (updated => compiler |> Compiler.incremental(updated))
       )
     );
};
