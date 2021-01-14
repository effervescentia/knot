open Kore;

module Default = {
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

type t('a) = {
  name: string,
  desc: string,
  spec: Arg.spec,
  alias: option(string),
  default: option(Default.t),
  options: option(list(string)),
};

let create =
    (
      ~alias=?,
      ~default=?,
      ~options=?,
      name: string,
      spec: Arg.spec,
      desc: string,
    )
    : t('a) => {
  name,
  spec,
  desc,
  alias,
  default,
  options,
};

let to_config = (value: t('a)): list((string, Arg.spec, string)) =>
  [(value.name |> Print.fmt("--%s"), value.spec, "")]
  @ (
    switch (value.alias) {
    | Some(alias) => [(alias |> Print.fmt("-%s"), value.spec, "")]
    | None => []
    }
  );

let to_string = (value: t('a)): string =>
  Print.fmt(
    "  %s%s%s\n    %s",
    value.name
    |> (
      switch (value.alias) {
      | Some(alias) => Print.fmt("-%s, --%s", alias)
      | None => Print.fmt("--%s")
      }
    )
    |> Print.bold,
    switch (value.default) {
    | Some(default) =>
      Default.to_string(default) |> Print.bold |> Print.fmt(" (default=%s)")
    | None => ""
    },
    switch (value.options) {
    | Some(options) =>
      Print.many(~separator=", ", Functional.identity, options)
      |> Print.bold
      |> Print.fmt(" (options=%s)")
    | None => ""
    },
    value.desc,
  );
