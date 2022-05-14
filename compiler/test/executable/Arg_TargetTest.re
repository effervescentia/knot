open Kore;

module Arg_Target = Executable.Arg_Target;

let __config = Config.defaults(false);

let suite =
  "Executable.Arg_Target"
  >::: [
    "create() - default"
    >: (
      () =>
        Assert.string(
          "-t, --target
  [options: javascript-es6, javascript-common, knot]
  the target to compile to",
          Arg_Target.create() |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "-t, --target
  [options: javascript-es6, javascript-common, knot]
  [from config: knot]
  the target to compile to",
          Arg_Target.create()
          |> fst
          |> ~@Argument.pp(Some({...__config, target: Some(Knot)})),
        )
    ),
    "create() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "-t, --target
  [options: javascript-es6, javascript-common, knot]
  [from config: knot]
  the target to compile to",
          Arg_Target.create()
          |> fst
          |> ~@Argument.pp(Some({...__config, target: Some(Knot)})),
        )
    ),
  ];
