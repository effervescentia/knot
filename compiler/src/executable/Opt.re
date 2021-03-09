/**
 Command line arg parsing utilities.
 */
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

type t = {
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
    : t => {
  name,
  spec,
  desc,
  alias,
  default,
  options,
};

let to_config = (value: t): list((string, Arg.spec, string)) =>
  [(value.name |> Print.fmt("--%s"), value.spec, "")]
  @ (
    switch (value.alias) {
    | Some(alias) => [(alias |> Print.fmt("-%s"), value.spec, "")]
    | None => []
    }
  );

let to_string = (value: t): string =>
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

module Shared = {
  let __targets = ["javascript"];
  let __min_port = 1024;
  let __max_port = 49151;

  let port = (~default=1337, ()) => {
    let value = ref(default);
    let opt =
      create(
        ~alias="p",
        ~default=Default.Int(default),
        "port",
        Arg.Set_int(value),
        "the port the server runs on",
      );
    let resolve = () => {
      let port = value^;

      if (port < __min_port || port > __max_port) {
        Print.fmt(
          "port must be in the range of %d to %d",
          __min_port,
          __max_port,
        )
        |> panic;
      };

      port;
    };

    (opt, resolve);
  };

  let root_dir = (~default="", ()) => {
    let value = ref(default);
    let opt =
      create(
        ~default=Default.String(default),
        "root-dir",
        Arg.Set_string(value),
        "the root directory to reference modules from",
      );
    let resolve = () => {
      let root_dir = value^;

      if (root_dir == "") {
        panic("root directory not provided");
      };

      if (!Sys.file_exists(root_dir)) {
        root_dir |> Print.fmt("root directory does not exist: '%s'") |> panic;
      };

      root_dir;
    };

    (opt, resolve);
  };

  let source_dir = (~default="", ()) => {
    let value = ref(default);
    let opt =
      create(
        ~default=Default.String(default),
        "source-dir",
        Arg.Set_string(value),
        Print.bold("root-dir")
        |> Print.fmt(
             "the directory to reference source modules from, relative to %s",
           ),
      );
    let resolve = () => value^;

    (opt, resolve);
  };

  let target = () => {
    let value = ref(None);
    let opt =
      create(
        ~alias="t",
        ~options=__targets,
        "target",
        Arg.Symbol(
          __targets,
          fun
          | "javascript" => value := Some(Target.JavaScript(Target.ES6))
          | x => Print.fmt("unknown target: '%s'", x) |> panic,
        ),
        "the target to compile to",
      );
    let resolve = () =>
      value^ |!: (() => panic("must provide a target for compilation"));

    (opt, resolve);
  };

  let out_dir = () => {
    let value = ref("");
    let opt =
      create(
        ~alias="o",
        "out-dir",
        Arg.Set_string(value),
        "the directory to write compiled files to",
      );
    let resolve = () => value^;

    (opt, resolve);
  };

  let compile = () => {
    let (root_dir_opt, get_root_dir) = root_dir(~default=Sys.getcwd(), ());
    let (source_dir_opt, get_source_dir) = source_dir(~default="src", ());

    let resolve = () => {
      let root_dir = get_root_dir();

      let source_dir = get_source_dir();
      let source_dir =
        Filename.is_implicit(source_dir) || Filename.is_relative(source_dir)
          ? Filename.concat(root_dir, source_dir) : source_dir;

      Log.error("source %s", source_dir);

      Compiler.{
        name: Filename.basename(root_dir),
        entry: Internal(""),
        root_dir,
        source_dir,
      };
    };

    ([root_dir_opt, source_dir_opt], resolve);
  };
};
