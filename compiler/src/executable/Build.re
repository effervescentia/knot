/**
 Utilities for the "build" command.
 */
open Kore;

type config_t = {
  target: Target.t,
  root_dir: string,
  source_dir: string,
  out_dir: string,
  entry: Namespace.t,
  fail_fast: bool,
  log_imports: bool,
};

let command_key = "build";

let command = () => {
  let (root_dir_arg, get_root_dir) = Arg_RootDir.create();
  let (source_dir_arg, get_source_dir) = Arg_SourceDir.create();
  let (out_dir_arg, get_out_dir) = Arg_OutDir.create();
  let (entry_arg, get_entry) = Arg_Entry.create();
  let (target_arg, get_target) = Arg_Target.create();
  let (fail_fast_arg, get_fail_fast) = Arg_FailFast.create();
  let (log_imports_arg, get_log_imports) = Arg_LogImports.create();

  Command.create(
    command_key,
    [
      root_dir_arg,
      source_dir_arg,
      out_dir_arg,
      entry_arg,
      target_arg,
      fail_fast_arg,
      log_imports_arg,
    ],
    (static, global) => {
      let root_dir = get_root_dir(static, global.working_dir);
      let source_dir = get_source_dir(static, root_dir);
      let out_dir = get_out_dir(static, root_dir);
      let entry = get_entry(static, source_dir.absolute);
      let target = get_target(static);
      let fail_fast = get_fail_fast(static);
      let log_imports = get_log_imports(static);

      {
        root_dir,
        source_dir: source_dir.relative,
        out_dir,
        entry,
        target,
        fail_fast,
        log_imports,
      };
    },
  );
};

let run = (global: Config.global_t, ~report=Reporter.panic, config: config_t) => {
  Util.log_config(
    global,
    command_key,
    [
      (root_dir_key, config.root_dir),
      (source_dir_key, config.source_dir),
      (out_dir_key, config.out_dir),
      (entry_key, config.entry |> ~@Namespace.pp),
      (target_key, config.target |> ~@Target.pp),
      (fail_fast_key, string_of_bool(config.fail_fast)),
      (log_imports_key, string_of_bool(config.log_imports)),
    ],
  );

  let compiler =
    Compiler.create(
      ~report,
      {
        name: global.name,
        root_dir: config.root_dir,
        source_dir: config.source_dir,
        fail_fast: config.fail_fast,
        log_imports: config.log_imports,
      },
    );

  compiler |> Compiler.compile(config.target, config.out_dir, config.entry);
  Compiler.teardown(compiler);
};
