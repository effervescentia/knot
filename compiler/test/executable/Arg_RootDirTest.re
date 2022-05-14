open Kore;

module Arg_RootDir = Executable.Arg_RootDir;

let __config = Config.defaults(false);

let suite =
  "Executable.Arg_RootDir"
  >::: [
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "-r, --root-dir
  [default: foo]
  the root directory to reference modules from",
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
  the root directory to reference modules from",
          Arg_RootDir.create(~default="foo", ())
          |> fst
          |> ~@Argument.pp(Some({...__config, root_dir: "bar"})),
        )
    ),
  ];
