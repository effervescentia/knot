/**
 Command argument parsing utilities.
 */
module Fmt = Pretty.Formatters;

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
  name: String.to_kebab_case(name),
  spec,
  desc,
  alias,
  default,
  from_config,
  options,
};

let expand = (value: t): list((string, Arg.spec, string)) => [
  (value.name |> Fmt.str("--%s"), value.spec, ""),
  ...switch (value.alias) {
     | Some(alias) => [(alias |> Fmt.str("-%s"), value.spec, "")]
     | None => []
     },
];

let resolve =
    (
      cfg: option(Config.t),
      select: Config.t => 'a,
      default: 'a,
      value: option('a),
    )
    : 'a =>
  switch (cfg, value) {
  | (_, Some(value)) => value
  | (Some(cfg), None) => select(cfg)
  | (None, None) => default
  };

/* pretty printing */

let _pp_flag: Fmt.t((string, option(string))) =
  (ppf, (name, alias)) =>
    switch (alias) {
    | Some(alias) => Fmt.pf(ppf, "-%s, --%s", alias, name)
    | None => Fmt.pf(ppf, "--%s", name)
    };

let _pp_attr = (label: string, pp_value: Fmt.t('a)): Fmt.t('a) =>
  Fmt.(ppf => pf(ppf, "[%s: %a]@,", label, bold(pp_value)));

let _pp_option_list: Fmt.t(option(list(string))) =
  ppf =>
    fun
    | Some(options) =>
      _pp_attr("options", Fmt.(list(~sep=Sep.comma, string)), ppf, options)
    | None => Fmt.nop(ppf, ());

let _pp_default: Fmt.t(option(Value.t)) =
  ppf =>
    fun
    | Some(default) => _pp_attr("default", Value.pp, ppf, default)
    | None => Fmt.nop(ppf, ());

let _pp_config = (opt: t): Fmt.t(option(Config.t)) =>
  ppf =>
    fun
    | Some(cfg) =>
      switch (opt.from_config(cfg), opt.default) {
      | (Some(from_config), Some(default)) when from_config == default =>
        Fmt.nop(ppf, ())
      | (Some(from_config), _) =>
        _pp_attr("from config", Value.pp, ppf, from_config)
      | _ => Fmt.nop(ppf, ())
      }
    | None => Fmt.nop(ppf, ());

let pp = (cfg: option(Config.t)): Fmt.t(t) =>
  Fmt.(
    vbox((ppf, value) =>
      pf(
        ppf,
        "%a%t@[<v>%a%a%a%s@]",
        bold(_pp_flag),
        (value.name, value.alias),
        indent,
        _pp_option_list,
        value.options,
        _pp_default,
        value.default,
        _pp_config(value),
        cfg,
        value.desc,
      )
    )
  );
