open Globals;
open KnotCompile.Compiler;

module Error = KnotDebug.Error;

let run = ({paths, main as in_path} as config, compiler) => {
  try(compiler.add(in_path)) {
  | ExecutionError(InvalidPathFormat(s) | ModuleDoesNotExist(_, s))
      when s == in_path =>
    throw_exec(InvalidEntryPoint(in_path))
  };

  switch (compiler.status()) {
  | Failing(errors) =>
    Log.error("%s  compilation failed!", Emoji.red_paper_lantern);

    List.iter(
      ((err, file, pretty_path)) =>
        Error.print_compilation_error(file, pretty_path, err),
      errors,
    );

    raise(InternalCompilationError);
  | _ =>
    compiler.complete();

    Log.info("%s  compiled!", Emoji.input_numbers);

    Writer.clean_dir(paths.build_dir);
    FileUtil.relative_path(paths.root_dir, paths.build_dir)
    |> Log.info("%s  (%s)", Emoji.sparkles);

    compiler.iter(in_path, paths.source_dir, Writer.write(config));

    Log.info("%s  done!", Emoji.confetti_ball);
  };
};
