open Kore;

module Arg_FailFast = Executable.Arg_FailFast;

let __config = Config.defaults(false);

let suite =
  "Executable.Arg_FailFast"
  >::: [
    "create() - default"
    >: (
      () =>
        Assert.string(
          "--fail-fast
  [default: false]
  fail as soon as the first error is encountered",
          Arg_FailFast.create() |> fst |> ~@Argument.pp(None),
        )
    ),
    "create() - overridden default"
    >: (
      () =>
        Assert.string(
          "--fail-fast
  [default: true]
  fail as soon as the first error is encountered",
          Arg_FailFast.create(~default=true, ())
          |> fst
          |> ~@Argument.pp(None),
        )
    ),
    "create() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "--fail-fast
  [default: false]
  [from config: true]
  fail as soon as the first error is encountered",
          Arg_FailFast.create()
          |> fst
          |> ~@Argument.pp(Some({...__config, fail_fast: true})),
        )
    ),
  ];
