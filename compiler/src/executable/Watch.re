/**
 Utilities for the "watch" command.
 */
open Kore;

module Namespace = Reference.Namespace;
module Watcher = File.Watcher;

type config_t = {
  target: Target.t,
  root_dir: string,
  source_dir: string,
  out_dir: string,
  entry: Namespace.t,
  ambient: string,
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
    (static, global, argv) => {
      let root_dir = get_root_dir(static, global.working_dir);
      let source_dir = get_source_dir(static, root_dir);
      let out_dir = get_out_dir(static, root_dir);
      let entry = get_entry(static, source_dir.absolute);
      let (target, ambient) = get_target(~argv, static);

      {
        root_dir,
        source_dir: source_dir.relative,
        out_dir,
        entry,
        target,
        ambient,
      };
    },
  );
};

let extract_config = (config: config_t) => [
  (root_dir_key, config.root_dir),
  (source_dir_key, config.source_dir),
  (out_dir_key, config.out_dir),
  (entry_key, config.entry |> ~@Namespace.pp),
  (target_key, config.target |> ~@Target.pp),
  (ambient_key, config.ambient),
];

let run =
    (
      global: Config.global_t,
      ~report=resolver =>
                Reporter.report(resolver) % File.Writer.write(stderr),
      config: config_t,
    ) => {
  Util.log_config(global, command_key, extract_config(config));

  let source_dir =
    config.source_dir |> Filename.resolve(~cwd=config.root_dir);
  let out_dir = config.out_dir |> Filename.resolve(~cwd=config.root_dir);
  let compiler =
    Compiler.create(
      ~report,
      {
        name: global.name,
        root_dir: config.root_dir,
        source_dir: config.source_dir,
        fail_fast: false,
        log_imports: false,
        stdlib: global.stdlib,
        ambient: config.ambient,
      },
    );

  Compiler.prepare(compiler);

  Sys.set_signal(
    Sys.sigterm,
    Sys.Signal_handle(_ => Compiler.teardown(compiler)),
  );

  Log.info("running initial compilation");

  compiler
  |> Compiler.compile(~skip_cache=true, config.target, out_dir, config.entry);

  let pp_source_relative = (ppf, relative) =>
    (relative, relative |> Filename.resolve(~cwd=source_dir))
    |> Fmt.captioned(ppf);
  let watcher = Watcher.create(source_dir);

  Log.info("%s", Fmt.str("starting file watcher") |> ~@Fmt.warn_str);
  Log.info(
    "watching for changes in %s",
    source_dir |> ~@Fmt.relative_path(global.working_dir),
  );

  watcher
  |> Watcher.(
       watch(actions =>
         actions
         |> List.map(((path, action)) => {
              let namespace = Namespace.of_path(path);

              File.FilesystemDriver.(
                switch (action) {
                | Add when _is_source_file(path) =>
                  Log.debug("file added %s", path |> ~@pp_source_relative);

                  compiler |> Compiler.upsert_module(namespace);

                | Update when _is_source_file(path) =>
                  Log.debug("file updated %s", path |> ~@pp_source_relative);

                  compiler |> Compiler.upsert_module(namespace);

                | Remove when _is_source_file(path) =>
                  Log.debug("file removed %s", path |> ~@pp_source_relative);

                  compiler |> Compiler.remove_module(namespace) |> snd;

                | _ => []
                }
              );
            })
         |> List.flatten
         |> (
           updated => {
             Log.info(
               "compiling %s module(s)",
               updated |> List.length |> ~@Fmt.(info(int)),
             );

             compiler |> Compiler.incremental(updated);

             updated
             |> List.iter(namespace =>
                  compiler
                  |> Compiler.get_module(namespace)
                  |> Option.iter(
                       Compiler.emit_one(config.target, out_dir, namespace),
                     )
                );

             Log.info("watching for more changes");
           }
         )
       )
     );
};
