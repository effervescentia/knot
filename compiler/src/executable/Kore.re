include Knot.Kore;
include Resolve.Kore;

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
