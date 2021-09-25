open Kore;

let __type_error =
  TypeError(Type.Error.NotFound(Reference.Identifier.Public("foo")));

let suite =
  "Library.Error"
  >::: [
    "parse_err_to_string()"
    >: (
      () =>
        [
          ("type error: NotFound<foo>", parse_err_to_string(__type_error)),
          (
            "reserved keyword foo cannot be used as an identifier",
            parse_err_to_string(ReservedKeyword("foo")),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "compile_err_to_string()"
    >: (
      () =>
        [
          (
            "import cycle between the following modules: foo -> bar -> fizz -> foo",
            compile_err_to_string(
              ImportCycle(["foo", "bar", "fizz", "foo"]),
            ),
          ),
          (
            "could not resolve module: foo",
            compile_err_to_string(UnresolvedModule("foo")),
          ),
          (
            "could not find file with path: foo",
            compile_err_to_string(FileNotFound("foo")),
          ),
          (
            "error found while parsing foo: type error: NotFound<foo>",
            compile_err_to_string(
              ParseError(
                __type_error,
                Reference.Namespace.of_string("foo"),
                Range.zero,
              ),
            ),
          ),
          (
            "failed to parse module: foo",
            compile_err_to_string(
              InvalidModule(Reference.Namespace.of_string("foo")),
            ),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "print_errs()"
    >: (
      () =>
        [
          (
            "found some errors during compilation:

could not resolve module: foo

could not resolve module: bar",
            print_errs([UnresolvedModule("foo"), UnresolvedModule("bar")]),
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
