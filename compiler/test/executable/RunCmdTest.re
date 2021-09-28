open Kore;

module RunCmd = Executable.RunCmd;

let suite =
  "Executable.RunCmd"
  >::: [
    "pp()"
    >: (
      () =>
        [
          (
            "build",
            RunCmd.Build({
              target: Target.Knot,
              entry: Reference.Namespace.External("foo"),
              source_dir: "source",
              out_dir: "output",
              fail_fast: true,
            })
            |> ~@RunCmd.pp,
          ),
          (
            "watch",
            RunCmd.Watch({
              target: Target.Knot,
              entry: Reference.Namespace.External("foo"),
              source_dir: "source",
              out_dir: "output",
            })
            |> ~@RunCmd.pp,
          ),
          ("format", RunCmd.Format({source_dir: "source"}) |> ~@RunCmd.pp),
          ("lint", RunCmd.Lint({fix: true}) |> ~@RunCmd.pp),
          ("lsp", RunCmd.LSP() |> ~@RunCmd.pp),
          (
            "bundle",
            RunCmd.Bundle({source_dir: "source", out_dir: "output"})
            |> ~@RunCmd.pp,
          ),
          ("develop", RunCmd.Develop({port: 8080}) |> ~@RunCmd.pp),
        ]
        |> Assert.(test_many(string))
    ),
  ];
