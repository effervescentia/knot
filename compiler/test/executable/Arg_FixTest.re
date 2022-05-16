open Kore;

module Arg_Fix = Executable.Arg_Fix;

let suite =
  "Executable.Arg_Fix"
  >::: [
    "create() - default"
    >: (
      () =>
        Assert.string(
          "--fix
  [default: false]
  automatically apply fixes",
          Arg_Fix.create() |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "--fix
  [default: true]
  automatically apply fixes",
          Arg_Fix.create(~default=true, ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "--fix
  [default: false]
  [from config: true]
  automatically apply fixes",
          Arg_Fix.create()
          |> fst
          |> ~@Argument.pp(Some({...Config.defaults, fix: true})),
        )
    ),
  ];
