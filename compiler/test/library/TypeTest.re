open Kore;

module T = Type;
module TR = Type.Raw;

let __args = [T.Valid(`Nil), T.Valid(`String)];
let __props = [("foo", T.Valid(`Nil)), ("bar", T.Valid(`String))];
let __raw_args = [TR.(`Nil), TR.(`String)];
let __raw_props = [("foo", TR.(`Nil)), ("bar", TR.(`String))];

let suite = [
  "Library.Type.Raw"
  >::: [
    "pp() - primitive" >: (() => Assert.string("bool", `Boolean |> ~@TR.pp)),
    "pp() - list"
    >: (() => Assert.string("List<string>", `List(`String) |> ~@TR.pp)),
    "pp() - struct"
    >: (
      () =>
        Assert.string(
          "{ foo: nil, bar: string }",
          `Struct(__raw_props) |> ~@TR.pp,
        )
    ),
    "pp() - function"
    >: (
      () =>
        Assert.string(
          "Function<(nil, string), bool>",
          `Function((__raw_args, `Boolean)) |> ~@TR.pp,
        )
    ),
  ],
  "Library.Type"
  >::: [
    "pp_valid() - primitive"
    >: (() => Assert.string("bool", `Boolean |> ~@T.pp_valid)),
    "pp_valid() - list"
    >: (
      () =>
        Assert.string(
          "List<string>",
          `List(T.Valid(`String)) |> ~@T.pp_valid,
        )
    ),
    "pp_valid() - struct"
    >: (
      () =>
        Assert.string(
          "{ foo: nil, bar: string }",
          `Struct(__props) |> ~@T.pp_valid,
        )
    ),
    "pp_valid() - function"
    >: (
      () =>
        Assert.string(
          "Function<(nil, string), bool>",
          `Function((__args, T.Valid(`Boolean))) |> ~@T.pp_valid,
        )
    ),
    "pp_invalid() - not inferrable"
    >: (
      () => Assert.string("NotInferrable", T.NotInferrable |> ~@T.pp_invalid)
    ),
    "pp_error() - not found"
    >: (
      () =>
        Assert.string(
          "NotFound<foo>",
          T.NotFound(Public("foo")) |> ~@T.pp_error,
        )
    ),
    "pp_error() - duplicate identifier"
    >: (
      () =>
        Assert.string(
          "DuplicateIdentifier<foo>",
          T.DuplicateIdentifier(Public("foo")) |> ~@T.pp_error,
        )
    ),
    "pp_error() - untyped function argument"
    >: (
      () =>
        Assert.string(
          "UntypedFunctionArgument<foo>",
          T.UntypedFunctionArgument(Public("foo")) |> ~@T.pp_error,
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
          T.TypeMismatch(Valid(`Nil), Valid(`String)) |> ~@T.pp_error,
        )
    ),
    "pp() - valid"
    >: (() => Assert.string("bool", T.Valid(`Boolean) |> ~@T.pp)),
  ],
];
