open Kore;
open Type;

let __props = [("foo", Valid(`Nil)), ("bar", Valid(`String))];
let __raw_props = [
  ("foo", Raw.Strong(`Nil)),
  ("bar", Raw.Strong(`String)),
];

let suite = [
  "Library.Type.Error"
  >::: [
    "pp() - not found"
    >: (
      () =>
        Assert.string(
          "NotFound<foo>",
          Error.NotFound(Public("foo")) |> ~@Error.pp(pp),
        )
    ),
    "pp() - duplicate identifier"
    >: (
      () =>
        Assert.string(
          "DuplicateIdentifier<foo>",
          Error.DuplicateIdentifier(Public("foo")) |> ~@Error.pp(pp),
        )
    ),
    "pp() - external not found"
    >: (
      () =>
        Assert.string(
          "ExternalNotFound<@/foo#main>",
          Error.ExternalNotFound(Internal("foo"), Main) |> ~@Error.pp(pp),
        )
    ),
    "pp() - type mismatch"
    >: (
      () =>
        Assert.string(
          "TypeMismatch<nil, string>",
          Error.TypeMismatch(Valid(`Nil), Valid(`String)) |> ~@Error.pp(pp),
        )
    ),
  ],
  "Library.Type.Raw"
  >::: [
    "pp_primitive() - nil"
    >: (() => Assert.string("nil", `Nil |> ~@Raw.pp_primitive)),
    "pp_primitive() - boolean"
    >: (() => Assert.string("bool", `Boolean |> ~@Raw.pp_primitive)),
    "pp_primitive() - integer"
    >: (() => Assert.string("int", `Integer |> ~@Raw.pp_primitive)),
    "pp_primitive() - float"
    >: (() => Assert.string("float", `Float |> ~@Raw.pp_primitive)),
    "pp_primitive() - string"
    >: (() => Assert.string("string", `String |> ~@Raw.pp_primitive)),
    "pp_primitive() - element"
    >: (() => Assert.string("element", `Element |> ~@Raw.pp_primitive)),
    "pp_list()"
    >: (
      () =>
        Assert.string("List<nil>", `Nil |> ~@Raw.pp_list(Raw.pp_primitive))
    ),
    "pp_abstract()"
    >: (
      () =>
        Assert.string("Abstract<Unknown>", Trait.Unknown |> ~@Raw.pp_abstract)
    ),
    "pp_struct() - empty"
    >: (() => Assert.string("{}", [] |> ~@Raw.pp_struct(pp))),
    "pp_struct() - with properties"
    >: (
      () =>
        Assert.string(
          "{ foo: nil, bar: string }",
          __props |> ~@Raw.pp_struct(pp),
        )
    ),
    "pp_function() - empty"
    >: (
      () =>
        Assert.string(
          "Function<(), bool>",
          ([], Valid(`Boolean)) |> ~@Raw.pp_function(pp),
        )
    ),
    "pp_function() - with argument and return types"
    >: (
      () =>
        Assert.string(
          "Function<(foo: nil, bar: string), int>",
          (__props, Valid(`Integer)) |> ~@Raw.pp_function(pp),
        )
    ),
    "pp_strong() - primitive"
    >: (() => Assert.string("bool", `Boolean |> ~@Raw.pp_strong)),
    "pp_strong() - list"
    >: (
      () =>
        Assert.string(
          "List<string>",
          `List(Raw.Strong(`String)) |> ~@Raw.pp_strong,
        )
    ),
    "pp_strong() - struct"
    >: (
      () =>
        Assert.string(
          "{ foo: nil, bar: string }",
          `Struct(__raw_props) |> ~@Raw.pp_strong,
        )
    ),
    "pp_strong() - function"
    >: (
      () =>
        Assert.string(
          "Function<(foo: nil, bar: string), bool>",
          `Function((__raw_props, Raw.Strong(`Boolean))) |> ~@Raw.pp_strong,
        )
    ),
    "pp_weak() - primitive"
    >: (() => Assert.string("bool", Ok(`Boolean) |> ~@Raw.pp_weak)),
    "pp_weak() - list"
    >: (
      () =>
        Assert.string(
          "List<string>",
          Ok(`List(Raw.Strong(`String))) |> ~@Raw.pp_weak,
        )
    ),
    "pp_weak() - struct"
    >: (
      () =>
        Assert.string(
          "{ foo: nil, bar: string }",
          Ok(`Struct(__raw_props)) |> ~@Raw.pp_weak,
        )
    ),
    "pp_weak() - function"
    >: (
      () =>
        Assert.string(
          "Function<(foo: nil, bar: string), bool>",
          Ok(`Function((__raw_props, Raw.Strong(`Boolean))))
          |> ~@Raw.pp_weak,
        )
    ),
    "pp_weak() - generic"
    >: (
      () =>
        Assert.string(
          "Generic<1, 3>",
          Ok(`Generic((1, 3))) |> ~@Raw.pp_weak,
        )
    ),
    "pp() - strong"
    >: (() => Assert.string("bool", Raw.Strong(`Boolean) |> ~@Raw.pp)),
    "pp() - weak"
    >: (() => Assert.string("Weak<2, 3>", Raw.Weak(2, 3) |> ~@Raw.pp)),
  ],
  "Library.Type"
  >::: [
    "pp_valid() - primitive"
    >: (() => Assert.string("bool", `Boolean |> ~@pp_valid)),
    "pp_valid() - list"
    >: (
      () =>
        Assert.string("List<string>", `List(Valid(`String)) |> ~@pp_valid)
    ),
    "pp_valid() - struct"
    >: (
      () =>
        Assert.string(
          "{ foo: nil, bar: string }",
          `Struct(__props) |> ~@pp_valid,
        )
    ),
    "pp_valid() - function"
    >: (
      () =>
        Assert.string(
          "Function<(foo: nil, bar: string), bool>",
          `Function((__props, Valid(`Boolean))) |> ~@pp_valid,
        )
    ),
    "pp_valid() - generic"
    >: (() => Assert.string("Generic<1, 3>", `Generic((1, 3)) |> ~@pp_valid)),
    "pp() - valid" >: (() => Assert.string("bool", Valid(`Boolean) |> ~@pp)),
  ],
];
