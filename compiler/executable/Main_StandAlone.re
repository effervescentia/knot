open Kore;
open Kore.Compiler;

let run = ({paths, main as in_path}, compiler) => {
  try (compiler.add(in_path)) {
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

    Writer.clean_build_dir();

    compiler.iter(in_path, paths.source_dir, Writer.write);

    Log.info("%s  done!", Emoji.confetti_ball);
  };
};
