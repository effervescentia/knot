/**
 Command utilities.
 */
open Kore;

type t =
  | Build(Build.config_t)
  | Watch(Watch.config_t)
  | Format(Format.config_t)
  | Lint(Lint.config_t)
  | Bundle(Bundle.config_t)
  | DevServe(DevServe.config_t)
  | LangServe(LangServe.config_t)
  | BuildServe(BuildServe.config_t);

let build = Build.cmd() |> Cmd.map(x => Build(x));
let watch = Watch.cmd() |> Cmd.map(x => Watch(x));
let format = Format.cmd() |> Cmd.map(x => Format(x));
let lint = Lint.cmd() |> Cmd.map(x => Lint(x));
let bundle = Bundle.cmd() |> Cmd.map(x => Bundle(x));
let dev_serve = DevServe.cmd() |> Cmd.map(x => DevServe(x));
let lang_serve = LangServe.cmd() |> Cmd.map(x => LangServe(x));
let build_serve = BuildServe.cmd() |> Cmd.map(x => BuildServe(x));

let commands = [
  (build_key, " compile files to target in output directory"),
  (watch_key, " run build and incrementally rebuild changed files"),
  (format_key, " update code style and spacing"),
  (lint_key, " analyze code style and report on anti-patterns"),
  (bundle_key, " generate executable from source code"),
  (
    dev_serve_key,
    " run a development server to enable continuous development",
  ),
  (lang_serve_key, " run an LSP-compliant server for integration with IDEs"),
  (
    build_serve_key,
    " run a JSONRPC server that can perform incremental compilation",
  ),
];

/* pretty printing */

let pp: Fmt.t(t) =
  ppf =>
    (
      fun
      | Build(_) => build_key
      | Watch(_) => watch_key
      | Format(_) => format_key
      | Lint(_) => lint_key
      | Bundle(_) => bundle_key
      | DevServe(_) => dev_serve_key
      | LangServe(_) => lang_serve_key
      | BuildServe(_) => build_serve_key
    )
    % Fmt.string(ppf);
