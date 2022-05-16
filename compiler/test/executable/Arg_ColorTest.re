open Kore;

module Arg_Color = Executable.Arg_Color;

let suite =
  "Executable.Arg_Color"
  >::: [
    "create() - default"
    >: (
      () =>
        Assert.string(
          Fmt.str("--color
  [default: false]
  allow color in logs"),
          Arg_Color.create() |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "--color
  [default: true]
  allow color in logs",
          Arg_Color.create(~default=true, ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          Fmt.str(
            "--color
  [default: false]
  [from config: %b]
  allow color in logs",
            true,
          ),
          Arg_Color.create()
          |> fst
          |> ~@Argument.pp(Some({...Config.defaults, color: true})),
        )
    ),
  ];
