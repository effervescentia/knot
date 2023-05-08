open Kore;

module Arg_RootDir = Executable.Arg_RootDir;

let suite =
  "Executable.Arg_RootDir"
  >::: [
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "-r, --root-dir
  [default: foo]
  the root directory used to resolve paths within the project",
          Arg_RootDir.create(~default="foo", ())
          |> fst
          |> ~@Argument.pp(None),
        )
    ),
    "create() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "-r, --root-dir
  [default: foo]
  [from config: bar]
  the root directory used to resolve paths within the project",
          Arg_RootDir.create(~default="foo", ())
          |> fst
          |> ~@Argument.pp(Some({...Config.defaults, root_dir: "bar"})),
        )
    ),
  ];
