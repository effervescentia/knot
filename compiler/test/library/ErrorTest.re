open Kore;

let __type_error =
  TypeError(Type.Error.NotFound(Reference.Identifier.Public("foo")));

let suite =
  "Library.Error"
  >::: [
    "pp_parse_err()"
    >: (
      () =>
        [
          ("type error: NotFound<foo>", __type_error |> ~@pp_parse_err),
          (
            "reserved keyword foo cannot be used as an identifier",
            ReservedKeyword("foo") |> ~@pp_parse_err,
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "pp_compile_err()"
    >: (
      () =>
        [
          (
            "import cycle between the following modules: foo -> bar -> fizz -> foo",
            ImportCycle(["foo", "bar", "fizz", "foo"]) |> ~@pp_compile_err,
          ),
          (
            "could not resolve module: foo",
            UnresolvedModule("foo") |> ~@pp_compile_err,
          ),
          (
            "could not find file with path: foo",
            FileNotFound("foo") |> ~@pp_compile_err,
          ),
          (
            "error found while parsing foo: type error: NotFound<foo>",
            ParseError(
              __type_error,
              Reference.Namespace.of_string("foo"),
              Range.zero,
            )
            |> ~@pp_compile_err,
          ),
          (
            "failed to parse module: foo",
            InvalidModule(Reference.Namespace.of_string("foo"))
            |> ~@pp_compile_err,
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "pp_err_list()"
    >: (
      () =>
        [
          (
            "found some errors during compilation:

could not resolve module: foo

could not resolve module: bar",
            [UnresolvedModule("foo"), UnresolvedModule("bar")]
            |> ~@pp_err_list,
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
