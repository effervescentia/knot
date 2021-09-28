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
            "  --debug
    [default: false]

    enable a higher level of logging",
            ConfigOpt.debug() |> fst |> ~@Opt.pp(None),
          ),
          (
            "  --debug
    [default: true]

    enable a higher level of logging",
            ConfigOpt.debug(~default=true, ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "  --debug
    [default: false]
    [from config: true]

    enable a higher level of logging",
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
            "  -r, --root-dir
    [default: foo]

    the root directory to reference modules from",
            ConfigOpt.root_dir(~default="foo", ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "  -r, --root-dir
    [default: foo]
    [from config: bar]

    the root directory to reference modules from",
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
            "  -p, --port
    [default: 1337]

    the port the server runs on",
            ConfigOpt.port() |> fst |> ~@Opt.pp(None),
          ),
          (
            "  -p, --port
    [default: 3000]

    the port the server runs on",
            ConfigOpt.port(~default=3000, ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "  -p, --port
    [default: 1337]
    [from config: 3000]

    the port the server runs on",
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
            "  -s, --source-dir
    [default: src]

    the directory to reference source modules from, relative to root-dir",
            ConfigOpt.source_dir() |> fst |> ~@Opt.pp(None),
          ),
          (
            "  -s, --source-dir
    [default: foo]

    the directory to reference source modules from, relative to root-dir",
            ConfigOpt.source_dir(~default="foo", ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "  -s, --source-dir
    [default: src]
    [from config: foo]

    the directory to reference source modules from, relative to root-dir",
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
            "  -e, --entry
    [default: main.kn]

    the entry point for execution, relative to source-dir",
            ConfigOpt.entry() |> fst |> ~@Opt.pp(None),
          ),
          (
            "  -e, --entry
    [default: foo]

    the entry point for execution, relative to source-dir",
            ConfigOpt.entry(~default="foo", ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "  -e, --entry
    [default: main.kn]
    [from config: foo]

    the entry point for execution, relative to source-dir",
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
            "  -t, --target (options: javascript-es6, javascript-common, knot)

    the target to compile to",
            ConfigOpt.target() |> fst |> ~@Opt.pp(None),
          ),
          (
            "  -t, --target (options: javascript-es6, javascript-common, knot)
    [from config: knot]

    the target to compile to",
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
            "  -o, --out-dir
    [default: build]

    the directory to write compiled files to",
            ConfigOpt.out_dir() |> fst |> ~@Opt.pp(None),
          ),
          (
            "  -o, --out-dir
    [default: foo]

    the directory to write compiled files to",
            ConfigOpt.out_dir(~default="foo", ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "  -o, --out-dir
    [default: build]
    [from config: bar]

    the directory to write compiled files to",
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
            "  --fix
    [default: false]

    automatically apply fixes",
            ConfigOpt.fix() |> fst |> ~@Opt.pp(None),
          ),
          (
            "  --fix
    [default: true]

    automatically apply fixes",
            ConfigOpt.fix(~default=true, ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "  --fix
    [default: false]
    [from config: true]

    automatically apply fixes",
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
            "  --color
    [default: true]

    allow color in logs",
            ConfigOpt.color() |> fst |> ~@Opt.pp(None),
          ),
          (
            "  --color
    [default: false]

    allow color in logs",
            ConfigOpt.color(~default=false, ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "  --color
    [default: true]
    [from config: false]

    allow color in logs",
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
            "  --fail-fast
    [default: false]

    fail as soon as the first error is encountered",
            ConfigOpt.fail_fast() |> fst |> ~@Opt.pp(None),
          ),
          (
            "  --fail-fast
    [default: true]

    fail as soon as the first error is encountered",
            ConfigOpt.fail_fast(~default=true, ()) |> fst |> ~@Opt.pp(None),
          ),
          (
            "  --fail-fast
    [default: false]
    [from config: true]

    fail as soon as the first error is encountered",
            ConfigOpt.fail_fast()
            |> fst
            |> ~@Opt.pp(Some({...__config, fail_fast: true})),
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
