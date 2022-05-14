open Kore;

module Arg_Entry = Executable.Arg_Entry;

let __config = Config.defaults(false);

let suite =
  "Executable.Arg_Entry"
  >::: [
    "create() - default"
    >: (
      () =>
        Assert.string(
          "-e, --entry
  [default: main.kn]
  the entry point for execution, relative to source dir",
          Arg_Entry.create() |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "-e, --entry
  [default: foo]
  the entry point for execution, relative to source dir",
          Arg_Entry.create(~default="foo", ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "-e, --entry
  [default: main.kn]
  [from config: foo]
  the entry point for execution, relative to source dir",
          Arg_Entry.create()
          |> fst
          |> ~@Argument.pp(Some({...__config, entry: "foo"})),
        )
    ),
  ];
