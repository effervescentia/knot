open Kore;
open Kore.Compiler;

let run = ({paths, main as in_path}, compiler) => {
  try (compiler.add(in_path)) {
  | InvalidPathFormat(s)
  | ModuleDoesNotExist(_, s) when s == in_path =>
    raise(InvalidEntryPoint(in_path))
  };

  compiler.complete();

  Log.info("%s  compiled!", Emoji.input_numbers);

  Writer.clean_build_dir();

  compiler.iter(in_path, paths.source_dir, Writer.write);

  Log.info("%s  done!", Emoji.confetti_ball);
};
