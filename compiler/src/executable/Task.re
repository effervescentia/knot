/**
 Task utilities.
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

let build = Build.command() |> Command.map(x => Build(x));
let watch = Watch.command() |> Command.map(x => Watch(x));
let format = Format.command() |> Command.map(x => Format(x));
let lint = Lint.command() |> Command.map(x => Lint(x));
let bundle = Bundle.command() |> Command.map(x => Bundle(x));
let dev_serve = DevServe.command() |> Command.map(x => DevServe(x));
let lang_serve = LangServe.command() |> Command.map(x => LangServe(x));
let build_serve = BuildServe.command() |> Command.map(x => BuildServe(x));

/* pretty printing */

let pp: Fmt.t(t) =
  ppf =>
    (
      fun
      | Build(_) => build.name
      | Watch(_) => watch.name
      | Format(_) => format.name
      | Lint(_) => lint.name
      | Bundle(_) => bundle.name
      | DevServe(_) => dev_serve.name
      | LangServe(_) => lang_serve.name
      | BuildServe(_) => build_serve.name
    )
    % Fmt.string(ppf);
