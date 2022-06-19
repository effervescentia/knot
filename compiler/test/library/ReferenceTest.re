open Kore;
open Reference;

let suite = [
  "Library.Reference.Namespace"
  >::: [
    "pp() - internal"
    >: (
      () =>
        Assert.string("@/foo", Namespace.Internal("foo") |> ~@Namespace.pp)
    ),
    "pp() - external"
    >: (
      () => Assert.string("foo", Namespace.External("foo") |> ~@Namespace.pp)
    ),
  ],
  "Library.Reference.Module"
  >::: [
    "pp() - root"
    >: (() => Assert.string("[root]", Module.Root |> ~@Module.pp)),
    "pp() - inner"
    >: (() => Assert.string("foo", Module.Inner("foo", None) |> ~@Module.pp)),
    "pp() - nested"
    >: (
      () =>
        Assert.string(
          "bar.foo",
          Module.Inner("foo", Some(Inner("bar", None))) |> ~@Module.pp,
        )
    ),
  ],
  "Library.Reference.Export"
  >::: [
    "pp() - main export"
    >: (() => Assert.string("main", Export.Main |> ~@Export.pp)),
    "pp() - public export"
    >: (() => Assert.string("foo", Export.Named("foo") |> ~@Export.pp)),
  ],
];
