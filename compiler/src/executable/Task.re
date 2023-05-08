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

let build = Build.command % Command.map(x => Build(x));
let watch = Watch.command % Command.map(x => Watch(x));
let format = Format.command % Command.map(x => Format(x));
let lint = Lint.command % Command.map(x => Lint(x));
let bundle = Bundle.command % Command.map(x => Bundle(x));
let dev_serve = DevServe.command % Command.map(x => DevServe(x));
let lang_serve = LangServe.command % Command.map(x => LangServe(x));
let build_serve = BuildServe.command % Command.map(x => BuildServe(x));

/* pretty printing */

let pp: Fmt.t(t) =
  ppf =>
    (
      fun
      | Build(cfg) => (Build.command_key, Build.extract_config(cfg))

      | Watch(cfg) => (Watch.command_key, Watch.extract_config(cfg))

      | Format(cfg) => (Format.command_key, Format.extract_config(cfg))

      | Lint(cfg) => (Lint.command_key, Lint.extract_config(cfg))

      | Bundle(cfg) => (Bundle.command_key, Bundle.extract_config(cfg))

      | DevServe(cfg) => (DevServe.command_key, DevServe.extract_config(cfg))

      | LangServe(cfg) => (
          LangServe.command_key,
          LangServe.extract_config(cfg),
        )

      | BuildServe(cfg) => (
          BuildServe.command_key,
          BuildServe.extract_config(cfg),
        )
    )
    % Tuple.map_fst2(String.to_pascal_case % Fmt.str("%sTask"))
    % Fmt.(struct_(string, string, ppf));
