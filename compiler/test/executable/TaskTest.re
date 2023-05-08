open Kore;

module Task = Executable.Task;

let suite =
  "Executable.Task"
  >::: [
    "pp() - build"
    >: (
      () =>
        Assert.string(
          "BuildTask {
  root_dir: root
  source_dir: source
  out_dir: output
  entry: foo
  target: knot
  fail_fast: true
  log_imports: false
  ambient: ambient.kd
}",
          Task.Build({
            root_dir: "root",
            source_dir: "source",
            out_dir: "output",
            entry: Reference.Namespace.External("foo"),
            target: Target.Knot,
            fail_fast: true,
            log_imports: false,
            ambient: "ambient.kd",
          })
          |> ~@Fmt.root(Task.pp),
        )
    ),
    "pp() - watch"
    >: (
      () =>
        Assert.string(
          "WatchTask {
  root_dir: root
  source_dir: source
  out_dir: output
  entry: foo
  target: knot
  ambient: ambient.kd
}",
          Task.Watch({
            root_dir: "root",
            source_dir: "source",
            out_dir: "output",
            entry: Reference.Namespace.External("foo"),
            target: Target.Knot,
            ambient: "ambient.kd",
          })
          |> ~@Fmt.root(Task.pp),
        )
    ),
    "pp() - format"
    >: (
      () =>
        Assert.string(
          "FormatTask {
  root_dir: root
  source_dir: source
  ambient: ambient.kd
}",
          Task.Format({
            root_dir: "root",
            source_dir: "source",
            ambient: "ambient.kd",
          })
          |> ~@Fmt.root(Task.pp),
        )
    ),
    "pp() - lint"
    >: (
      () =>
        Assert.string(
          "LintTask {
  root_dir: root
  fix: true
}",
          Task.Lint({root_dir: "root", fix: true}) |> ~@Fmt.root(Task.pp),
        )
    ),
    "pp() - bundle"
    >: (
      () =>
        Assert.string(
          "BundleTask {
  root_dir: root
  source_dir: source
  out_dir: output
}",
          Task.Bundle({
            root_dir: "root",
            source_dir: "source",
            out_dir: "output",
          })
          |> ~@Fmt.root(Task.pp),
        )
    ),
    "pp() - lang_serve"
    >: (
      () =>
        Assert.string(
          "LangServeTask { }",
          Task.LangServe() |> ~@Fmt.root(Task.pp),
        )
    ),
    "pp() - build_serve"
    >: (
      () =>
        Assert.string(
          "BuildServeTask {
  root_dir: root
  source_dir: source
  target: knot
  log_imports: false
  ambient: ambient.kd
}",
          Task.BuildServe({
            root_dir: "root",
            source_dir: "source",
            target: Target.Knot,
            log_imports: false,
            ambient: "ambient.kd",
          })
          |> ~@Fmt.root(Task.pp),
        )
    ),
    "pp() - dev_serve"
    >: (
      () =>
        Assert.string(
          "DevServeTask {
  port: 8080
}",
          Task.DevServe({port: 8080}) |> ~@Fmt.root(Task.pp),
        )
    ),
  ];
