open Kore;

module Arg_SourceDir = Executable.Arg_SourceDir;

let suite =
  "Executable.Arg_SourceDir"
  >::: [
    "create() - default"
    >: (
      () =>
        Assert.string(
          "-s, --source-dir
  [default: src]
  the directory to reference source modules from, relative to root dir",
          Arg_SourceDir.create() |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "-s, --source-dir
  [default: foo]
  the directory to reference source modules from, relative to root dir",
          Arg_SourceDir.create(~default="foo", ())
          |> fst
          |> ~@Argument.pp(None),
        )
    ),
    "create() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "-s, --source-dir
  [default: src]
  [from config: foo]
  the directory to reference source modules from, relative to root dir",
          Arg_SourceDir.create()
          |> fst
          |> ~@Argument.pp(Some({...Config.defaults, source_dir: "foo"})),
        )
    ),
  ];
