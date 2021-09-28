open Kore;
open Reference;

let suite = [
  "Library.Reference.Namespace"
  >::: [
    "pp()"
    >: (
      () =>
        [
          ("@/foo", Namespace.Internal("foo") |> ~@Namespace.pp),
          ("foo", Namespace.External("foo") |> ~@Namespace.pp),
        ]
        |> Assert.(test_many(string))
    ),
  ],
  "Library.Reference.Module"
  >::: [
    "pp()"
    >: (
      () =>
        [
          ("[root]", Module.Root |> ~@Module.pp),
          ("foo", Module.Inner("foo", None) |> ~@Module.pp),
          (
            "bar.foo",
            Module.Inner("foo", Some(Inner("bar", None))) |> ~@Module.pp,
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ],
  "Library.Reference.Identifier"
  >::: [
    "pp()"
    >: (
      () =>
        [
          ("foo", Identifier.Public("foo") |> ~@Identifier.pp),
          ("_foo", Identifier.Private("foo") |> ~@Identifier.pp),
        ]
        |> Assert.(test_many(string))
    ),
  ],
  "Library.Reference.Export"
  >::: [
    "pp()"
    >: (
      () =>
        [
          ("main", Export.Main |> ~@Export.pp),
          ("foo", Export.Named(Public("foo")) |> ~@Export.pp),
          ("_foo", Export.Named(Private("foo")) |> ~@Export.pp),
        ]
        |> Assert.(test_many(string))
    ),
  ],
];
