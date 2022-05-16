open Kore;

module Arg_LogImports = Executable.Arg_LogImports;

let suite =
  "Executable.Arg_LogImports"
  >::: [
    "create() - default"
    >: (
      () =>
        Assert.string(
          "--log-imports
  [default: false]
  print a graph describing the dependencies between modules",
          Arg_LogImports.create() |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "--log-imports
  [default: true]
  print a graph describing the dependencies between modules",
          Arg_LogImports.create(~default=true, ())
          |> fst
          |> ~@Argument.pp(None),
        )
    ),
    "create() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "--log-imports
  [default: false]
  [from config: true]
  print a graph describing the dependencies between modules",
          Arg_LogImports.create()
          |> fst
          |> ~@Argument.pp(Some({...Config.defaults, log_imports: true})),
        )
    ),
  ];
