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

let _map = (f, (name, opts, resolve): (string, list(Opt.t), unit => 'a)) => (
  name,
  opts,
  () => f(resolve()),
);

let of_build = _map(x => Build(x));
let of_watch = _map(x => Watch(x));
let of_format = _map(x => Format(x));
let of_lint = _map(x => Lint(x));
let of_lsp = _map(x => LSP(x));
let of_bundle = _map(x => Bundle(x));
let of_develop = _map(x => Develop(x));
