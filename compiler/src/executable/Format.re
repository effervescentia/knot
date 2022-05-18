/**
 Utilities for the "format" command.
 */
open Kore;

type config_t = {
  root_dir: string,
  source_dir: string,
};

let command_key = "format";

let command = () => {
  let (root_dir_arg, get_root_dir) = Arg_RootDir.create();
  let (source_dir_arg, get_source_dir) = Arg_SourceDir.create();

  Command.create(
    command_key,
    [root_dir_arg, source_dir_arg],
    (static, global) => {
      let root_dir = get_root_dir(static, global.working_dir);
      let source_dir = get_source_dir(static, root_dir).relative;

      {root_dir, source_dir};
    },
  );
};

let extract_config = (config: config_t) => [
  (root_dir_key, config.root_dir),
  (source_dir_key, config.source_dir),
];

let run = (global: Config.global_t, ~report=Reporter.panic, config: config_t) => {
  Util.log_config(global, command_key, extract_config(config));

  let pp_relative = Fmt.relative_path(global.working_dir);
  let target_dir =
    config.source_dir |> Filename.resolve(~cwd=config.root_dir);
  let compiler =
    Compiler.create(
      ~report=
        resolver =>
          List.filter(
            fun
            | ImportCycle(_)
            | UnresolvedModule(_)
            | FileNotFound(_)
            | ParseError(_)
            | InvalidModule(_) => false,
          )
          % (errors => List.is_empty(errors) ? () : report(resolver, errors)),
      {
        name: global.name,
        root_dir: config.root_dir,
        source_dir: config.source_dir,
        fail_fast: false,
        log_imports: false,
      },
    );

  Log.info("reading modules from %s", target_dir |> ~@pp_relative);

  let files =
    FileUtil.find(
      And(
        Is_file,
        And(
          Has_extension(String.sub(Constants.file_extension, 1, 2)),
          And(Is_readable, Is_writeable),
        ),
      ),
      target_dir,
      (acc, path) => [path, ...acc],
      [],
    );

  files
  |> List.iter(
       ~@Fmt.relative_path(target_dir) % Log.debug("formatting file %s"),
     );

  let modules =
    files
    |> List.map(
         Filename.relative_to(target_dir)
         % String.drop_suffix(Constants.file_extension)
         % Namespace.of_internal,
       );

  compiler
  |> Compiler.process(modules, Compiler.resolve(~skip_cache=true, compiler));
  compiler |> Compiler.emit(Target.Knot, target_dir);

  Log.info(
    "formatted %s file(s)",
    files |> List.length |> ~@Fmt.(info(int)),
  );
  Log.info("%s", "done!" |> ~@Fmt.good_str);

  Compiler.teardown(compiler);
};
