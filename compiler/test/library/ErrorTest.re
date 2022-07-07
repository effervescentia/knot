open Kore;

let __type_error = TypeError(NotFound("foo"));

let suite =
  "Library.Error"
  >::: [
    "pp_parse_err() - type error"
    >: (
      () =>
        Assert.string(
          "type error: NotFound<foo>",
          __type_error |> ~@pp_parse_err,
        )
    ),
    "pp_parse_err() - reserved keyword"
    >: (
      () =>
        Assert.string(
          "reserved keyword foo cannot be used as an identifier",
          ReservedKeyword("foo") |> ~@pp_parse_err,
        )
    ),
    "pp_dump_parse_err() - type error"
    >: (
      () =>
        Assert.string(
          "TypeError<NotFound<foo>>",
          __type_error |> ~@pp_dump_parse_err,
        )
    ),
    "pp_dump_parse_err() - reserved keyword"
    >: (
      () =>
        Assert.string(
          "ReservedKeyword<foo>",
          ReservedKeyword("foo") |> ~@pp_dump_parse_err,
        )
    ),
    "pp_compile_err() - import cycle"
    >: (
      () =>
        Assert.string(
          "import cycle found between modules foo -> bar -> fizz -> foo",
          ImportCycle(["foo", "bar", "fizz", "foo"]) |> ~@pp_compile_err,
        )
    ),
    "pp_compile_err() - unresolved module"
    >: (
      () =>
        Assert.string(
          "could not resolve module foo",
          UnresolvedModule("foo") |> ~@pp_compile_err,
        )
    ),
    "pp_compile_err() - file not found"
    >: (
      () =>
        Assert.string(
          "could not find file with path foo",
          FileNotFound("foo") |> ~@pp_compile_err,
        )
    ),
    "pp_compile_err() - parse error"
    >: (
      () =>
        Assert.string(
          "error found while parsing foo: type error: NotFound<foo>",
          ParseError(
            __type_error,
            Reference.Namespace.of_string("foo"),
            Range.zero,
          )
          |> ~@pp_compile_err,
        )
    ),
    "pp_compile_err() - invalid module"
    >: (
      () =>
        Assert.string(
          "failed to parse module foo",
          InvalidModule(Reference.Namespace.of_string("foo"))
          |> ~@pp_compile_err,
        )
    ),
    "pp_dump_compile_err() - import cycle"
    >: (
      () =>
        Assert.string(
          "ImportCycle<foo -> bar -> fizz -> foo>",
          ImportCycle(["foo", "bar", "fizz", "foo"]) |> ~@pp_dump_compile_err,
        )
    ),
    "pp_dump_compile_err() - unresolved module"
    >: (
      () =>
        Assert.string(
          "UnresolvedModule<foo>",
          UnresolvedModule("foo") |> ~@pp_dump_compile_err,
        )
    ),
    "pp_dump_compile_err() - file not found"
    >: (
      () =>
        Assert.string(
          "FileNotFound<foo>",
          FileNotFound("foo") |> ~@pp_dump_compile_err,
        )
    ),
    "pp_dump_compile_err() - parse error"
    >: (
      () =>
        Assert.string(
          "ParseError<TypeError<NotFound<foo>>, foo, 0.0>",
          ParseError(
            __type_error,
            Reference.Namespace.of_string("foo"),
            Range.zero,
          )
          |> ~@pp_dump_compile_err,
        )
    ),
    "pp_dump_compile_err() - invalid module"
    >: (
      () =>
        Assert.string(
          "InvalidModule<foo>",
          InvalidModule(Reference.Namespace.of_string("foo"))
          |> ~@pp_dump_compile_err,
        )
    ),
    "pp_err_list()"
    >: (
      () =>
        Assert.string(
          "found some errors during compilation:

  could not resolve module foo
  \n  could not resolve module bar",
          [UnresolvedModule("foo"), UnresolvedModule("bar")]
          |> ~@pp_err_list,
        )
    ),
    "pp_dump_err_list()"
    >: (
      () =>
        Assert.string(
          "UnresolvedModule<foo>, UnresolvedModule<bar>",
          [UnresolvedModule("foo"), UnresolvedModule("bar")]
          |> ~@pp_dump_err_list,
        )
    ),
  ];
