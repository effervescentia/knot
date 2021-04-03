/**
 Command line arg parsing utilities.
 */
open Kore;

module Value = {
  type t =
    | Bool(bool)
    | Int(int)
    | String(string);

  let to_string =
    fun
    | Bool(x) => string_of_bool(x)
    | Int(x) => string_of_int(x)
    | String(x) => x;
};

type t = {
  name: string,
  desc: string,
  spec: Arg.spec,
  alias: option(string),
  default: option(Value.t),
  from_config: static_t => option(Value.t),
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
  name: name |> _to_kebab_case,
  spec,
  desc,
  alias,
  default,
  from_config,
  options,
};

let to_args = (value: t): list((string, Arg.spec, string)) => [
  (value.name |> Print.fmt("--%s"), value.spec, ""),
  ...switch (value.alias) {
     | Some(alias) => [(alias |> Print.fmt("-%s"), value.spec, "")]
     | None => []
     },
];

let to_string = (cfg: option(static_t), value: t): string =>
  Print.fmt(
    "  %s%s%s%s\n\n    %s",
    value.name
    |> (
      switch (value.alias) {
      | Some(alias) => Print.fmt("-%s, --%s", alias)
      | None => Print.fmt("--%s")
      }
    )
    |> Print.bold,
    switch (value.options) {
    | Some(options) =>
      Print.many(~separator=", ", Functional.identity, options)
      |> Print.bold
      |> Print.fmt(" (options: %s)")
    | None => ""
    },
    switch (value.default) {
    | Some(default) =>
      Value.to_string(default)
      |> Print.bold
      |> Print.fmt("\n    [default: %s]")
    | None => ""
    },
    switch (cfg) {
    | Some(cfg) =>
      switch (value.from_config(cfg), value.default) {
      | (Some(from_config), Some(default)) when from_config != default =>
        Value.to_string(from_config)
        |> Print.bold
        |> Print.fmt("\n    [from config: %s]")
      | _ => ""
      }
    | None => ""
    },
    value.desc,
  );
