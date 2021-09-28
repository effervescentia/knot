/**
 Command line arg parsing utilities.
 */
open Kore;

module Value = {
  type t =
    | Bool(bool)
    | Int(int)
    | String(string);

  /* pretty printing */

  let pp: Fmt.t(t) =
    ppf =>
      fun
      | Bool(x) => Fmt.bool(ppf, x)
      | Int(x) => Fmt.int(ppf, x)
      | String(x) => Fmt.string(ppf, x);
};

type t = {
  name: string,
  desc: string,
  spec: Arg.spec,
  alias: option(string),
  default: option(Value.t),
  from_config: Config.t => option(Value.t),
  options: option(list(string)),
};

let _to_kebab_case = String.replace('_', '-');

let create =
    (
      ~alias=?,
      ~default=?,
      ~from_config=_ => None,
      ~options=?,
      name: string,
      spec: Arg.spec,
      desc: string,
    )
    : t => {
  name: _to_kebab_case(name),
  spec,
  desc,
  alias,
  default,
  from_config,
  options,
};

let to_args = (value: t): list((string, Arg.spec, string)) => [
  (value.name |> Fmt.str("--%s"), value.spec, ""),
  ...switch (value.alias) {
     | Some(alias) => [(alias |> Fmt.str("-%s"), value.spec, "")]
     | None => []
     },
];

/* pretty printing */

let _pp_flag: Fmt.t((string, option(string))) =
  (ppf, (name, alias)) =>
    switch (alias) {
    | Some(alias) => Fmt.pf(ppf, "-%s, --%s", alias, name)
    | None => Fmt.pf(ppf, "--%s", name)
    };

let _pp_option_list: Fmt.t(option(list(string))) =
  ppf =>
    fun
    | Some(options) =>
      Fmt.pf(
        ppf,
        " (options: %a)",
        Fmt.bold(
          Fmt.list(~sep=(ppf, ()) => Fmt.string(ppf, ", "), Fmt.string),
        ),
        options,
      )
    | None => Fmt.nop(ppf, ());

let _pp_attr = (label: string): Fmt.t(Value.t) =>
  ppf => Fmt.pf(ppf, "\n    [%s: %a]", label, Fmt.bold(Value.pp));

let _pp_default: Fmt.t(option(Value.t)) =
  ppf =>
    fun
    | Some(default) => _pp_attr("default", ppf, default)
    | None => Fmt.nop(ppf, ());

let _pp_config = (opt: t): Fmt.t(option(Config.t)) =>
  ppf =>
    fun
    | Some(cfg) =>
      switch (opt.from_config(cfg), opt.default) {
      | (Some(from_config), Some(default)) when from_config == default =>
        Fmt.nop(ppf, ())
      | (Some(from_config), _) => _pp_attr("from config", ppf, from_config)
      | _ => Fmt.nop(ppf, ())
      }
    | None => Fmt.nop(ppf, ());

let pp = (cfg: option(Config.t)): Fmt.t(t) =>
  (ppf, value: t) =>
    Fmt.pf(
      ppf,
      "  %a%a%a%a\n\n    %s",
      Fmt.bold(_pp_flag),
      (value.name, value.alias),
      _pp_option_list,
      value.options,
      _pp_default,
      value.default,
      _pp_config(value),
      cfg,
      value.desc,
    );
