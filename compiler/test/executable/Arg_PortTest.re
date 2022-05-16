open Kore;

module Arg_Port = Executable.Arg_Port;

let suite =
  "Executable.Arg_Port"
  >::: [
    "create() - default"
    >: (
      () =>
        Assert.string(
          "-p, --port
  [default: 1337]
  the port the server runs on",
          Arg_Port.create() |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "-p, --port
  [default: 3000]
  the port the server runs on",
          Arg_Port.create(~default=3000, ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "-p, --port
  [default: 1337]
  [from config: 3000]
  the port the server runs on",
          Arg_Port.create()
          |> fst
          |> ~@Argument.pp(Some({...Config.defaults, port: 3000})),
        )
    ),
  ];
