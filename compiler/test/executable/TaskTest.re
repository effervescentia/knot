open Kore;

module Task = Executable.Task;

let suite =
  "Executable.Task"
  >::: [
    "pp() - build"
    >: (
      () =>
        Assert.string(
          "build",
          Task.Build({
            root_dir: "root",
            source_dir: "source",
            out_dir: "output",
            entry: Reference.Namespace.External("foo"),
            target: Target.Knot,
            fail_fast: true,
            log_imports: false,
          })
          |> ~@Task.pp,
        )
    ),
    "pp() - watch"
    >: (
      () =>
        Assert.string(
          "watch",
          Task.Watch({
            root_dir: "root",
            source_dir: "source",
            out_dir: "output",
            entry: Reference.Namespace.External("foo"),
            target: Target.Knot,
          })
          |> ~@Task.pp,
        )
    ),
    "pp() - format"
    >: (
      () =>
        Assert.string(
          "format",
          Task.Format({root_dir: "root", source_dir: "source"}) |> ~@Task.pp,
        )
    ),
    "pp() - lint"
    >: (
      () =>
        Assert.string(
          "lint",
          Task.Lint({root_dir: "root", fix: true}) |> ~@Task.pp,
        )
    ),
    "pp() - bundle"
    >: (
      () =>
        Assert.string(
          "bundle",
          Task.Bundle({
            root_dir: "root",
            source_dir: "source",
            out_dir: "output",
          })
          |> ~@Task.pp,
        )
    ),
    "pp() - lang_serve"
    >: (() => Assert.string("lang_serve", Task.LangServe() |> ~@Task.pp)),
    "pp() - build_serve"
    >: (
      () =>
        Assert.string(
          "build_serve",
          Task.BuildServe({
            root_dir: "root",
            source_dir: "source",
            target: Target.Knot,
          })
          |> ~@Task.pp,
        )
    ),
    "pp() - dev_serve"
    >: (
      () =>
        Assert.string("dev_serve", Task.DevServe({port: 8080}) |> ~@Task.pp)
    ),
  ];
