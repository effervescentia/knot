/**
 Utilities for the "format" command.
 */
open Kore;

type config_t = unit;

let cmd = () => {
  Cmd.create(format_key, [], (_, _) => ());
};

let run = (~report=print_errs % panic, global: global_t, config: config_t) => {
  Cmd.log_config(global, format_key, []);

  let compiler =
    Compiler.create(
      ~report=
        List.filter(
          fun
          | ImportCycle(_)
          | UnresolvedModule(_)
          | FileNotFound(_)
          | ParseError(_) => false,
        )
        % (errors => errors |> List.is_empty ? () : report(errors)),
      {
        name: global.name,
        root_dir: global.root_dir,
        source_dir: global.source_dir,
        fail_fast: false,
      },
    );

  let source_path = Filename.concat(global.root_dir, global.source_dir);

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
       Filename.concat(global.root_dir, global.source_dir),
     );
  compiler |> Compiler.teardown;
};
