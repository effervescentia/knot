open Kore;

module Arguments = Executable.Arguments;

let is_ci_env = Executable.Kore.is_ci_env;

let __config = Config.defaults(false);

let suite =
  "Executable.Arguments"
  >::: [
    "debug() - default"
    >: (
      () =>
        Assert.string(
          "--debug
  [default: false]
  \n  enable a higher level of logging",
          Arguments.debug() |> fst |> ~@Argument.pp(None),
        )
    ),
    "debug() - overridden default"
    >: (
      () =>
        Assert.string(
          "--debug
  [default: true]
  \n  enable a higher level of logging",
          Arguments.debug(~default=true, ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "debug() - inherited from config"
    >: (
      () =>
        Assert.string(
          "--debug
  [default: false]
  [from config: true]
  \n  enable a higher level of logging",
          Arguments.debug()
          |> fst
          |> ~@Argument.pp(Some({...__config, debug: true})),
        )
    ),
    "root_dir() - overridden default"
    >: (
      () =>
        Assert.string(
          "-r, --root-dir
  [default: foo]
  \n  the root directory to reference modules from",
          Arguments.root_dir(~default="foo", ())
          |> fst
          |> ~@Argument.pp(None),
        )
    ),
    "root_dir() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "-r, --root-dir
  [default: foo]
  [from config: bar]
  \n  the root directory to reference modules from",
          Arguments.root_dir(~default="foo", ())
          |> fst
          |> ~@Argument.pp(Some({...__config, root_dir: "bar"})),
        )
    ),
    "port() - default"
    >: (
      () =>
        Assert.string(
          "-p, --port
  [default: 1337]
  \n  the port the server runs on",
          Arguments.port() |> fst |> ~@Argument.pp(None),
        )
    ),
    "port() - overridden default"
    >: (
      () =>
        Assert.string(
          "-p, --port
  [default: 3000]
  \n  the port the server runs on",
          Arguments.port(~default=3000, ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "port() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "-p, --port
  [default: 1337]
  [from config: 3000]
  \n  the port the server runs on",
          Arguments.port()
          |> fst
          |> ~@Argument.pp(Some({...__config, port: 3000})),
        )
    ),
    "source_dir() - default"
    >: (
      () =>
        Assert.string(
          "-s, --source-dir
  [default: src]
  \n  the directory to reference source modules from, relative to root-dir",
          Arguments.source_dir() |> fst |> ~@Argument.pp(None),
        )
    ),
    "source_dir() - overridden default"
    >: (
      () =>
        Assert.string(
          "-s, --source-dir
  [default: foo]
  \n  the directory to reference source modules from, relative to root-dir",
          Arguments.source_dir(~default="foo", ())
          |> fst
          |> ~@Argument.pp(None),
        )
    ),
    "source_dir() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "-s, --source-dir
  [default: src]
  [from config: foo]
  \n  the directory to reference source modules from, relative to root-dir",
          Arguments.source_dir()
          |> fst
          |> ~@Argument.pp(Some({...__config, source_dir: "foo"})),
        )
    ),
    "entry() - default"
    >: (
      () =>
        Assert.string(
          "-e, --entry
  [default: main.kn]
  \n  the entry point for execution, relative to source-dir",
          Arguments.entry() |> fst |> ~@Argument.pp(None),
        )
    ),
    "entry() - overridden default"
    >: (
      () =>
        Assert.string(
          "-e, --entry
  [default: foo]
  \n  the entry point for execution, relative to source-dir",
          Arguments.entry(~default="foo", ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "entry() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "-e, --entry
  [default: main.kn]
  [from config: foo]
  \n  the entry point for execution, relative to source-dir",
          Arguments.entry()
          |> fst
          |> ~@Argument.pp(Some({...__config, entry: "foo"})),
        )
    ),
    "target() - default"
    >: (
      () =>
        Assert.string(
          "-t, --target
  [options: javascript-es6, javascript-common, knot]
  \n  the target to compile to",
          Arguments.target() |> fst |> ~@Argument.pp(None),
        )
    ),
    "target() - overridden default"
    >: (
      () =>
        Assert.string(
          "-t, --target
  [options: javascript-es6, javascript-common, knot]
  [from config: knot]
  \n  the target to compile to",
          Arguments.target()
          |> fst
          |> ~@Argument.pp(Some({...__config, target: Some(Knot)})),
        )
    ),
    "target() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "-t, --target
  [options: javascript-es6, javascript-common, knot]
  [from config: knot]
  \n  the target to compile to",
          Arguments.target()
          |> fst
          |> ~@Argument.pp(Some({...__config, target: Some(Knot)})),
        )
    ),
    "out_dir() - default"
    >: (
      () =>
        Assert.string(
          "-o, --out-dir
  [default: build]
  \n  the directory to write compiled files to",
          Arguments.out_dir() |> fst |> ~@Argument.pp(None),
        )
    ),
    "out_dir() - overridden default"
    >: (
      () =>
        Assert.string(
          "-o, --out-dir
  [default: foo]
  \n  the directory to write compiled files to",
          Arguments.out_dir(~default="foo", ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "out_dir() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "-o, --out-dir
  [default: build]
  [from config: bar]
  \n  the directory to write compiled files to",
          Arguments.out_dir()
          |> fst
          |> ~@Argument.pp(Some({...__config, out_dir: "bar"})),
        )
    ),
    "fix() - default"
    >: (
      () =>
        Assert.string(
          "--fix
  [default: false]
  \n  automatically apply fixes",
          Arguments.fix() |> fst |> ~@Argument.pp(None),
        )
    ),
    "fix() - overridden default"
    >: (
      () =>
        Assert.string(
          "--fix
  [default: true]
  \n  automatically apply fixes",
          Arguments.fix(~default=true, ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "fix() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "--fix
  [default: false]
  [from config: true]
  \n  automatically apply fixes",
          Arguments.fix()
          |> fst
          |> ~@Argument.pp(Some({...__config, fix: true})),
        )
    ),
    "color() - default"
    >: (
      () =>
        Assert.string(
          Fmt.str(
            "--color
  [default: %b]
  \n  allow color in logs",
            !is_ci_env,
          ),
          Arguments.color() |> fst |> ~@Argument.pp(None),
        )
    ),
    "color() - overridden default"
    >: (
      () =>
        Assert.string(
          "--color
  [default: false]
  \n  allow color in logs",
          Arguments.color(~default=false, ()) |> fst |> ~@Argument.pp(None),
        )
    ),
    "color() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          Fmt.str(
            "--color
  [default: %b]
  [from config: %b]
  \n  allow color in logs",
            !is_ci_env,
            is_ci_env,
          ),
          Arguments.color()
          |> fst
          |> ~@Argument.pp(Some({...__config, color: is_ci_env})),
        )
    ),
    "fail_fast() - default"
    >: (
      () =>
        Assert.string(
          "--fail-fast
  [default: false]
  \n  fail as soon as the first error is encountered",
          Arguments.fail_fast() |> fst |> ~@Argument.pp(None),
        )
    ),
    "fail_fast() - overridden default"
    >: (
      () =>
        Assert.string(
          "--fail-fast
  [default: true]
  \n  fail as soon as the first error is encountered",
          Arguments.fail_fast(~default=true, ())
          |> fst
          |> ~@Argument.pp(None),
        )
    ),
    "fail_fast() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "--fail-fast
  [default: false]
  [from config: true]
  \n  fail as soon as the first error is encountered",
          Arguments.fail_fast()
          |> fst
          |> ~@Argument.pp(Some({...__config, fail_fast: true})),
        )
    ),
    "log_imports() - default"
    >: (
      () =>
        Assert.string(
          "--log-imports
  [default: false]
  \n  print a graph describing the dependencies between modules",
          Arguments.log_imports() |> fst |> ~@Argument.pp(None),
        )
    ),
    "log_imports() - overridden default"
    >: (
      () =>
        Assert.string(
          "--log-imports
  [default: true]
  \n  print a graph describing the dependencies between modules",
          Arguments.log_imports(~default=true, ())
          |> fst
          |> ~@Argument.pp(None),
        )
    ),
    "log_imports() - with value inherited from config"
    >: (
      () =>
        Assert.string(
          "--log-imports
  [default: false]
  [from config: true]
  \n  print a graph describing the dependencies between modules",
          Arguments.log_imports()
          |> fst
          |> ~@Argument.pp(Some({...__config, log_imports: true})),
        )
    ),
  ];
