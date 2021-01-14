include Knot.Kore;
include Resolve.Kore;
include Generate.Kore;

module Compiler = Compile.Compiler;

module Mode = {
  type t =
    | Build
    | Watch
    | Format
    | Lint
    | LSP
    | Bundle
    | Develop;

  let to_string =
    fun
    | Build => "build"
    | Watch => "watch"
    | Format => "format"
    | Lint => "lint"
    | LSP => "lsp"
    | Bundle => "bundle"
    | Develop => "develop";
};

let panic = (err: string) => {
  Log.fatal("%s", err);

  exit(2);
};
