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
    "pp()"
    >: (
      () =>
        [
          (
            "NotFound<foo>",
            Error.NotFound(Public("foo")) |> ~@Error.pp(pp),
          ),
          (
            "DuplicateIdentifier<foo>",
            Error.DuplicateIdentifier(Public("foo")) |> ~@Error.pp(pp),
          ),
          (
            "NotAssignable<nil, Unknown>",
            Error.NotAssignable(Valid(`Nil), Trait.Unknown)
            |> ~@Error.pp(pp),
          ),
          (
            "ExternalNotFound<@/foo#main>",
            Error.ExternalNotFound(Internal("foo"), Main) |> ~@Error.pp(pp),
          ),
          (
            "TypeResolutionFailed",
            Error.TypeResolutionFailed |> ~@Error.pp(pp),
          ),
          (
            "TypeMismatch<nil, string>",
            Error.TypeMismatch(Valid(`Nil), Valid(`String))
            |> ~@Error.pp(pp),
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ],
  "Library.Type.Raw"
  >::: [
    "pp_primitive()"
    >: (
      () =>
        [
          ("nil", `Nil |> ~@Raw.pp_primitive),
          ("bool", `Boolean |> ~@Raw.pp_primitive),
          ("int", `Integer |> ~@Raw.pp_primitive),
          ("float", `Float |> ~@Raw.pp_primitive),
          ("string", `String |> ~@Raw.pp_primitive),
          ("element", `Element |> ~@Raw.pp_primitive),
        ]
        |> Assert.(test_many(string))
    ),
    "pp_list()"
    >: (
      () =>
        [("List<nil>", `Nil |> ~@Raw.pp_list(Raw.pp_primitive))]
        |> Assert.(test_many(string))
    ),
    "pp_abstract()"
    >: (
      () =>
        [("Abstract<Unknown>", Trait.Unknown |> ~@Raw.pp_abstract)]
        |> Assert.(test_many(string))
    ),
    "pp_struct()"
    >: (
      () =>
        [
          ("{}", [] |> ~@Raw.pp_struct(pp)),
          ("{ foo: nil, bar: string }", __props |> ~@Raw.pp_struct(pp)),
        ]
        |> Assert.(test_many(string))
    ),
    "pp_function()"
    >: (
      () =>
        [
          (
            "Function<(), bool>",
            ([], Valid(`Boolean)) |> ~@Raw.pp_function(pp),
          ),
          (
            "Function<(foo: nil, bar: string), int>",
            (__props, Valid(`Integer)) |> ~@Raw.pp_function(pp),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "pp_strong()"
    >: (
      () =>
        [
          ("bool", `Boolean |> ~@Raw.pp_strong),
          ("List<string>", `List(Raw.Strong(`String)) |> ~@Raw.pp_strong),
          (
            "{ foo: nil, bar: string }",
            `Struct(__raw_props) |> ~@Raw.pp_strong,
          ),
          (
            "Function<(foo: nil, bar: string), bool>",
            `Function((__raw_props, Raw.Strong(`Boolean))) |> ~@Raw.pp_strong,
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "pp_weak()"
    >: (
      () =>
        [
          ("bool", Ok(`Boolean) |> ~@Raw.pp_weak),
          (
            "List<string>",
            Ok(`List(Raw.Strong(`String))) |> ~@Raw.pp_weak,
          ),
          (
            "{ foo: nil, bar: string }",
            Ok(`Struct(__raw_props)) |> ~@Raw.pp_weak,
          ),
          (
            "Function<(foo: nil, bar: string), bool>",
            Ok(`Function((__raw_props, Raw.Strong(`Boolean))))
            |> ~@Raw.pp_weak,
          ),
          ("Generic<3>", Ok(`Generic(3)) |> ~@Raw.pp_weak),
          /* (
               "Abstract<Unknown>",
               Ok(`Abstract(Trait.Unknown)) |> ~@Raw.pp_weak,
             ), */
          (
            "TypeResolutionFailed",
            Error(Error.TypeResolutionFailed) |> ~@Raw.pp_weak,
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "pp()"
    >: (
      () =>
        [
          ("bool", Raw.Strong(`Boolean) |> ~@Raw.pp),
          (
            "TypeResolutionFailed",
            Raw.Invalid(TypeResolutionFailed) |> ~@Raw.pp,
          ),
          ("Weak<string>", Raw.Weak({contents: Ok(`String)}) |> ~@Raw.pp),
        ]
        |> Assert.(test_many(string))
    ),
  ],
  "Library.Type"
  >::: [
    "pp_valid()"
    >: (
      () =>
        [
          ("bool", `Boolean |> ~@pp_valid),
          ("List<string>", `List(Valid(`String)) |> ~@pp_valid),
          ("{ foo: nil, bar: string }", `Struct(__props) |> ~@pp_valid),
          (
            "Function<(foo: nil, bar: string), bool>",
            `Function((__props, Valid(`Boolean))) |> ~@pp_valid,
          ),
          /* ("Abstract<Unknown>", `Abstract(Trait.Unknown) |> ~@pp_valid), */
          ("Generic<3>", `Generic(3) |> ~@pp_valid),
        ]
        |> Assert.(test_many(string))
    ),
    "pp()"
    >: (
      () =>
        [
          ("bool", Valid(`Boolean) |> ~@pp),
          ("List<string>", Valid(`List(Valid(`String))) |> ~@pp),
          ("{ foo: nil, bar: string }", Valid(`Struct(__props)) |> ~@pp),
          (
            "Function<(foo: nil, bar: string), bool>",
            Valid(`Function((__props, Valid(`Boolean)))) |> ~@pp,
          ),
          /* ("Abstract<Unknown>", Valid(`Abstract(Trait.Unknown)) |> ~@pp), */
          ("Generic<3>", Valid(`Generic(3)) |> ~@pp),
          (
            "TypeResolutionFailed",
            Invalid(Error.TypeResolutionFailed) |> ~@pp,
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ],
];
