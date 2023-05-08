open Kore;

module T = AST.Type;

let __args = [T.Valid(Nil), T.Valid(String)];
let __props = [
  ("foo", (T.Valid(Nil), true)),
  ("bar", (T.Valid(String), false)),
];
let __raw_args = T.[Nil, String];
let __raw_props = T.[("foo", Nil), ("bar", String)];

let suite =
  "Library.Type"
  >::: [
    "pp_valid() - primitive"
    >: (() => Assert.string("boolean", Boolean |> ~@T.pp_valid)),
    "pp_valid() - list"
    >: (
      () => Assert.string("string[]", List(T.Valid(String)) |> ~@T.pp_valid)
    ),
    "pp_valid() - struct"
    >: (
      () =>
        Assert.string(
          "{ foo: nil, bar?: string }",
          Object(__props) |> ~@T.pp_valid,
        )
    ),
    "pp_valid() - function"
    >: (
      () =>
        Assert.string(
          "(nil, string) -> boolean",
          T.Function(__args, Valid(Boolean)) |> ~@T.pp_valid,
        )
    ),
    "pp_invalid() - not inferrable"
    >: (
      () => Assert.string("NotInferrable", T.NotInferrable |> ~@T.pp_invalid)
    ),
    "pp_error() - not found"
    >: (
      () => Assert.string("NotFound<foo>", T.NotFound("foo") |> ~@T.pp_error)
    ),
    "pp_error() - duplicate identifier"
    >: (
      () =>
        Assert.string(
          "DuplicateIdentifier<foo>",
          T.DuplicateIdentifier("foo") |> ~@T.pp_error,
        )
    ),
    "pp_error() - untyped function argument"
    >: (
      () =>
        Assert.string(
          "UntypedFunctionArgument<foo>",
          T.UntypedFunctionArgument("foo") |> ~@T.pp_error,
        )
    ),
    "pp() - external not found"
    >: (
      () =>
        Assert.string(
          "ExternalNotFound<@/foo#main>",
          T.ExternalNotFound(Internal("foo"), Main) |> ~@T.pp_error,
        )
    ),
    "pp() - type mismatch"
    >: (
      () =>
        Assert.string(
          "TypeMismatch<nil, string>",
          T.TypeMismatch(Valid(Nil), Valid(String)) |> ~@T.pp_error,
        )
    ),
    "pp() - valid"
    >: (() => Assert.string("boolean", T.Valid(Boolean) |> ~@T.pp)),
  ];
