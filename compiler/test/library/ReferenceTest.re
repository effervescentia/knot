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
  "Library.Reference.Export"
  >::: [
    "pp() - main export"
    >: (() => Assert.string("main", Export.Main |> ~@Export.pp)),
    "pp() - public export"
    >: (() => Assert.string("foo", Export.Named("foo") |> ~@Export.pp)),
  ],
];
