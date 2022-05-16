open Kore;

module Arg_Debug = Executable.Arg_Debug;

let suite =
  "Executable.Arg_Debug"
  >::: [
    "create() - default"
    >: (
      () =>
        Assert.string(
          "--debug
  [default: false]
  enable a higher level of logging",
          Arg_Debug.create() |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "--debug
  [default: true]
  enable a higher level of logging",
          Arg_Debug.create(~default=true, ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - inherited from config"
    >: (
      () =>
        Assert.string(
          "--debug
  [default: false]
  [from config: true]
  enable a higher level of logging",
          Arg_Debug.create()
          |> fst
          |> ~@Argument.pp(Some({...Config.defaults, debug: true})),
        )
    ),
  ];
