open Kore;

module RunCmd = Executable.RunCmd;

let suite =
  "Executable.RunCmd"
  >::: [
    "pp() - build"
    >: (
      () =>
        Assert.string(
          "build",
          RunCmd.Build({
            target: Target.Knot,
            entry: Reference.Namespace.External("foo"),
            source_dir: "source",
            out_dir: "output",
            fail_fast: true,
          })
          |> ~@RunCmd.pp,
        )
    ),
    "pp() - watch"
    >: (
      () =>
        Assert.string(
          "watch",
          RunCmd.Watch({
            target: Target.Knot,
            entry: Reference.Namespace.External("foo"),
            source_dir: "source",
            out_dir: "output",
          })
          |> ~@RunCmd.pp,
        )
    ),
    "pp() - format"
    >: (
      () =>
        Assert.string(
          "format",
          RunCmd.Format({source_dir: "source"}) |> ~@RunCmd.pp,
        )
    ),
    "pp() - lint"
    >: (() => Assert.string("lint", RunCmd.Lint({fix: true}) |> ~@RunCmd.pp)),
    "pp() - lsp" >: (() => Assert.string("lsp", RunCmd.LSP() |> ~@RunCmd.pp)),
    "pp() - bundle"
    >: (
      () =>
        Assert.string(
          "bundle",
          RunCmd.Bundle({source_dir: "source", out_dir: "output"})
          |> ~@RunCmd.pp,
        )
    ),
    "pp() - develop"
    >: (
      () =>
        Assert.string("develop", RunCmd.Develop({port: 8080}) |> ~@RunCmd.pp)
    ),
  ];
