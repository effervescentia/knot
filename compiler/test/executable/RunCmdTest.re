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
            log_imports: false,
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
    "pp() - lang_serve"
    >: (() => Assert.string("lang_serve", RunCmd.LangServe() |> ~@RunCmd.pp)),
    "pp() - bundle"
    >: (
      () =>
        Assert.string(
          "bundle",
          RunCmd.Bundle({source_dir: "source", out_dir: "output"})
          |> ~@RunCmd.pp,
        )
    ),
    "pp() - dev_serve"
    >: (
      () =>
        Assert.string(
          "dev_serve",
          RunCmd.DevServe({port: 8080}) |> ~@RunCmd.pp,
        )
    ),
  ];
