open Kore;

module Opt = Executable.Opt;

let __config = Config.defaults(false);
let __opt =
  Opt.create(
    "foo",
    Arg.Bool(ignore),
    "used to control the application of foo",
  );

let suite =
  "Executable.Opt"
  >::: [
    "pp()"
    >: (
      () =>
        [
          (
            "  --foo

    used to control the application of foo",
            Opt.create(
              "foo",
              Arg.Bool(ignore),
              "used to control the application of foo",
            )
            |> ~@Opt.pp(None),
          ),
          (
            "  -f, --foo (options: fizz, buzz)
    [default: true]
    [from config: false]

    used to control the application of foo",
            Opt.create(
              ~alias="f",
              ~default=Bool(true),
              ~from_config=_ => Some(Bool(false)),
              ~options=["fizz", "buzz"],
              "foo",
              Arg.Bool(ignore),
              "used to control the application of foo",
            )
            |> ~@Opt.pp(Some(__config)),
          ),
          (
            "  --foo
    [default: true]

    used to control the application of foo",
            Opt.create(
              ~default=Bool(true),
              ~from_config=_ => Some(Bool(true)),
              "foo",
              Arg.Bool(ignore),
              "used to control the application of foo",
            )
            |> ~@Opt.pp(Some(__config)),
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
