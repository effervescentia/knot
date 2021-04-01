/**
 Command utilities.
 */
open Kore;

type t =
  | Build(Build.config_t)
  | Watch(Watch.config_t)
  | Format(Format.config_t)
  | Lint(Lint.config_t)
  | LSP(LSP.config_t)
  | Bundle(Bundle.config_t)
  | Develop(Develop.config_t);

let to_string =
  fun
  | Build(_) => "build"
  | Watch(_) => "watch"
  | Format(_) => "format"
  | Lint(_) => "lint"
  | LSP(_) => "lsp"
  | Bundle(_) => "bundle"
  | Develop(_) => "develop";

let of_build = Mode.map(x => Build(x));
let of_watch = Mode.map(x => Watch(x));
let of_format = Mode.map(x => Format(x));
let of_lint = Mode.map(x => Lint(x));
let of_lsp = Mode.map(x => LSP(x));
let of_bundle = Mode.map(x => Bundle(x));
let of_develop = Mode.map(x => Develop(x));
