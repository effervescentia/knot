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
  | Build(_) => build_key
  | Watch(_) => watch_key
  | Format(_) => format_key
  | Lint(_) => lint_key
  | LSP(_) => lsp_key
  | Bundle(_) => bundle_key
  | Develop(_) => develop_key;

let build = Build.cmd() |> Cmd.map(x => Build(x));
let watch = Watch.cmd() |> Cmd.map(x => Watch(x));
let format = Format.cmd() |> Cmd.map(x => Format(x));
let lint = Lint.cmd() |> Cmd.map(x => Lint(x));
let lsp = LSP.cmd() |> Cmd.map(x => LSP(x));
let bundle = Bundle.cmd() |> Cmd.map(x => Bundle(x));
let develop = Develop.cmd() |> Cmd.map(x => Develop(x));
