open Kore;

let __opt =
  Argument.create(
    "foo",
    Arg.Bool(ignore),
    "used to control the application of foo",
  );

let suite =
  "Library.Argument"
  >::: [
    "pp() - with description"
    >: (
      () =>
        Assert.string(
          "--foo
  used to control the application of foo",
          Argument.create(
            "foo",
            Arg.Bool(ignore),
            "used to control the application of foo",
          )
          |> ~@Argument.pp(None),
        )
    ),
    "pp() - with attribute"
    >: (
      () =>
        Assert.string(
          "--foo
  [default: true]
  used to control the application of foo",
          Argument.create(
            ~default=Bool(true),
            ~from_config=_ => Some(Bool(true)),
            "foo",
            Arg.Bool(ignore),
            "used to control the application of foo",
          )
          |> ~@Argument.pp(Some(Config.defaults)),
        )
    ),
    "pp() - with many attributes and alias"
    >: (
      () =>
        Assert.string(
          "-f, --foo
  [options: fizz, buzz]
  [default: true]
  [from config: false]
  used to control the application of foo",
          Argument.create(
            ~alias="f",
            ~default=Bool(true),
            ~from_config=_ => Some(Bool(false)),
            ~options=["fizz", "buzz"],
            "foo",
            Arg.Bool(ignore),
            "used to control the application of foo",
          )
          |> ~@Argument.pp(Some(Config.defaults)),
        )
    ),
  ];
