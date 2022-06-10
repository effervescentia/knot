open Kore;

module Arg_OutDir = Executable.Arg_OutDir;

let suite =
  "Executable.Arg_OutDir"
  >::: [
    "create() - default"
    >: (
      () =>
        Assert.string(
          "-o, --out-dir
  [default: build]
  the directory to write compiled files to",
          Arg_OutDir.create() |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "-o, --out-dir
  [default: foo]
  the directory to write compiled files to",
          Arg_OutDir.create(~default="foo", ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "-o, --out-dir
  [default: build]
  [from config: bar]
  the directory to write compiled files to",
          Arg_OutDir.create()
          |> fst
          |> ~@Argument.pp(Some({...Config.defaults, out_dir: "bar"})),
        )
    ),
  ];
