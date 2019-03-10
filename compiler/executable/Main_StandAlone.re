open Kore;
open Kore.Compiler;

let run = ({paths, main as in_path}, compiler) => {
  try (compiler.add_rec(in_path)) {
  | InvalidPathFormat(s)
  | ModuleDoesNotExist(_, s) when s == in_path =>
    raise(InvalidEntryPoint(in_path))
  };

  if (compiler.is_complete()) {
    Writer.clean_build_dir();

    compiler.iter(in_path, paths.source_dir, Writer.write);

    Log.info("%s  done!", Emoji.confetti_ball);
  } else {
    Log.error(
      "%s",
      "failed to compile, the following statements could not be resolved:",
    );

    compiler.iter_pending(Debug.print_resolve_target % Log.error("%s"));

    exit(-1);
  };
};
