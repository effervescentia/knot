open Kore;

module Arg_Color = Executable.Arg_Color;

let __is_ci_env = Executable.Kore.is_ci_env;
let __config = Config.defaults(false);

let suite =
  "Executable.Arg_Color"
  >::: [
    "create() - default"
    >: (
      () =>
        Assert.string(
          Fmt.str(
            "--color
  [default: %b]
  allow color in logs",
            !__is_ci_env,
          ),
          Arg_Color.create() |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "--color
  [default: false]
  allow color in logs",
          Arg_Color.create(~default=false, ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          Fmt.str(
            "--color
  [default: %b]
  [from config: %b]
  allow color in logs",
            !__is_ci_env,
            __is_ci_env,
          ),
          Arg_Color.create()
          |> fst
          |> ~@Argument.pp(Some({...__config, color: __is_ci_env})),
        )
    ),
  ];
