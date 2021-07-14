/**
 Utilities for the "format" command.
 */
open Kore;

type config_t = {source_dir: string};

let cmd = () => {
  let (source_dir_opt, get_source_dir) = ConfigOpt.source_dir();

  Cmd.create(
    format_key,
    [source_dir_opt],
    (static, global) => {
      let source_dir = get_source_dir(static, global.root_dir);

      {source_dir: source_dir};
    },
  );
};

let run = (global: global_t, ~report=Reporter.panic, config: config_t) => {
  Cmd.log_config(global, format_key, []);

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
          % (
            errors => errors |> List.is_empty ? () : report(resolver, errors)
          ),
      {
        name: global.name,
        root_dir: global.root_dir,
        source_dir: config.source_dir,
        fail_fast: false,
      },
    );

  let source_path = Filename.concat(global.root_dir, config.source_dir);

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
       Filename.concat(global.root_dir, config.source_dir),
     );
  compiler |> Compiler.teardown;
};
