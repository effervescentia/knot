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
  let (root_dir_arg, get_root_dir) = Arguments.root_dir();
  let (source_dir_arg, get_source_dir) = Arguments.source_dir();

  Command.create(
    command_key,
    [root_dir_arg, source_dir_arg],
    (static, global) => {
      let root_dir = get_root_dir(static);
      let source_dir = get_source_dir(static, root_dir);

      {root_dir, source_dir};
    },
  );
};

let run = (global: Config.global_t, ~report=Reporter.panic, config: config_t) => {
  Util.log_config(
    global,
    command_key,
    [(root_dir_key, config.root_dir), (source_dir_key, config.source_dir)],
  );

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

  let source_path = Filename.concat(config.root_dir, config.source_dir);

  let files =
    FileUtil.find(
      And(
        Is_file,
        And(
          Has_extension(String.sub(Constants.file_extension, 1, 2)),
          And(Is_readable, Is_writeable),
        ),
      ),
      source_path,
      (acc, path) => [path, ...acc],
      [],
    )
    |> List.map(Filename.relative_to(source_path));

  files |> List.iter(Log.debug("formatting file: %s"));

  let modules =
    files
    |> List.map(path =>
         Namespace.Internal(
           path |> String.drop_suffix(Constants.file_extension),
         )
       );

  compiler
  |> Compiler.process(modules, Compiler.resolve(~skip_cache=true, compiler));
  compiler
  |> Compiler.emit_output(
       Target.Knot,
       Filename.concat(config.root_dir, config.source_dir),
     );
  compiler |> Compiler.teardown;
};
