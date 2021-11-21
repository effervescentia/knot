open Kore;

module Opt = Executable.Opt;
module ConfigOpt = Executable.ConfigOpt;

let __config = Config.defaults(false);

let suite =
  "Executable.ConfigOpt"
  >::: [
    "debug() - pretty printing"
    >: (
      () =>
        [
          (
            "--debug
  [default: false]
  \n  enable a higher level of logging",
            ConfigOpt.debug() |> fst |> ~@Opt.pp(None),
          ),
          (
            "--debug
  [default: true]
  \n  enable a higher level of logging",
            ConfigOpt.debug(~default=true, ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "--debug
  [default: false]
  [from config: true]
  \n  enable a higher level of logging",
            ConfigOpt.debug()
            |> fst
            |> ~@Opt.pp(Some({...__config, debug: true})),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "root_dir() - pretty printing"
    >: (
      () =>
        [
          (
            "-r, --root-dir
  [default: foo]
  \n  the root directory to reference modules from",
            ConfigOpt.root_dir(~default="foo", ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "-r, --root-dir
  [default: foo]
  [from config: bar]
  \n  the root directory to reference modules from",
            ConfigOpt.root_dir(~default="foo", ())
            |> fst
            |> ~@Opt.pp(Some({...__config, root_dir: "bar"})),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "port() - pretty printing"
    >: (
      () =>
        [
          (
            "-p, --port
  [default: 1337]
  \n  the port the server runs on",
            ConfigOpt.port() |> fst |> ~@Opt.pp(None),
          ),
          (
            "-p, --port
  [default: 3000]
  \n  the port the server runs on",
            ConfigOpt.port(~default=3000, ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "-p, --port
  [default: 1337]
  [from config: 3000]
  \n  the port the server runs on",
            ConfigOpt.port()
            |> fst
            |> ~@Opt.pp(Some({...__config, port: 3000})),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "source_dir() - pretty printing"
    >: (
      () =>
        [
          (
            "-s, --source-dir
  [default: src]
  \n  the directory to reference source modules from, relative to root-dir",
            ConfigOpt.source_dir() |> fst |> ~@Opt.pp(None),
          ),
          (
            "-s, --source-dir
  [default: foo]
  \n  the directory to reference source modules from, relative to root-dir",
            ConfigOpt.source_dir(~default="foo", ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "-s, --source-dir
  [default: src]
  [from config: foo]
  \n  the directory to reference source modules from, relative to root-dir",
            ConfigOpt.source_dir()
            |> fst
            |> ~@Opt.pp(Some({...__config, source_dir: "foo"})),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "entry() - pretty printing"
    >: (
      () =>
        [
          (
            "-e, --entry
  [default: main.kn]
  \n  the entry point for execution, relative to source-dir",
            ConfigOpt.entry() |> fst |> ~@Opt.pp(None),
          ),
          (
            "-e, --entry
  [default: foo]
  \n  the entry point for execution, relative to source-dir",
            ConfigOpt.entry(~default="foo", ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "-e, --entry
  [default: main.kn]
  [from config: foo]
  \n  the entry point for execution, relative to source-dir",
            ConfigOpt.entry()
            |> fst
            |> ~@Opt.pp(Some({...__config, entry: "foo"})),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "target() - pretty printing"
    >: (
      () =>
        [
          (
            "-t, --target
  [options: javascript-es6, javascript-common, knot]
  \n  the target to compile to",
            ConfigOpt.target() |> fst |> ~@Opt.pp(None),
          ),
          (
            "-t, --target
  [options: javascript-es6, javascript-common, knot]
  [from config: knot]
  \n  the target to compile to",
            ConfigOpt.target()
            |> fst
            |> ~@Opt.pp(Some({...__config, target: Some(Knot)})),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "out_dir() - pretty printing"
    >: (
      () =>
        [
          (
            "-o, --out-dir
  [default: build]
  \n  the directory to write compiled files to",
            ConfigOpt.out_dir() |> fst |> ~@Opt.pp(None),
          ),
          (
            "-o, --out-dir
  [default: foo]
  \n  the directory to write compiled files to",
            ConfigOpt.out_dir(~default="foo", ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "-o, --out-dir
  [default: build]
  [from config: bar]
  \n  the directory to write compiled files to",
            ConfigOpt.out_dir()
            |> fst
            |> ~@Opt.pp(Some({...__config, out_dir: "bar"})),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "fix() - pretty printing"
    >: (
      () =>
        [
          (
            "--fix
  [default: false]
  \n  automatically apply fixes",
            ConfigOpt.fix() |> fst |> ~@Opt.pp(None),
          ),
          (
            "--fix
  [default: true]
  \n  automatically apply fixes",
            ConfigOpt.fix(~default=true, ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "--fix
  [default: false]
  [from config: true]
  \n  automatically apply fixes",
            ConfigOpt.fix()
            |> fst
            |> ~@Opt.pp(Some({...__config, fix: true})),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "color() - pretty printing"
    >: (
      () =>
        [
          (
            "--color
  [default: true]
  \n  allow color in logs",
            ConfigOpt.color() |> fst |> ~@Opt.pp(None),
          ),
          (
            "--color
  [default: false]
  \n  allow color in logs",
            ConfigOpt.color(~default=false, ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "--color
  [default: true]
  [from config: false]
  \n  allow color in logs",
            ConfigOpt.color()
            |> fst
            |> ~@Opt.pp(Some({...__config, color: false})),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "fail_fast() - pretty printing"
    >: (
      () =>
        [
          (
            "--fail-fast
  [default: false]
  \n  fail as soon as the first error is encountered",
            ConfigOpt.fail_fast() |> fst |> ~@Opt.pp(None),
          ),
          (
            "--fail-fast
  [default: true]
  \n  fail as soon as the first error is encountered",
            ConfigOpt.fail_fast(~default=true, ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "--fail-fast
  [default: false]
  [from config: true]
  \n  fail as soon as the first error is encountered",
            ConfigOpt.fail_fast()
            |> fst
            |> ~@Opt.pp(Some({...__config, fail_fast: true})),
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
