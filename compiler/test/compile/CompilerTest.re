open Kore;
open Reference;

module Compiler = Compile.Compiler;
module ImportGraph = Resolve.ImportGraph;
module Source = Resolve.Source;
module U = Util.ResultUtil;
module A = AST;
module T = Type;
module Fx = Fixtures;
module Px = Fixtures.Program;
module Nx = Fixtures.Namespace;
module Cx = Fixtures.Compiler;
module Ox = Fixtures.Output;

let __entry_filename = "entry.kn";

let __scope_tree = BinaryTree.create((Range.zero, None));

let __types =
  [(Export.Named("ABC"), T.Valid(`Integer))]
  |> List.to_seq
  |> Hashtbl.of_seq;

let _create_module = (root_dir, namespace) =>
  Source.File({
    relative: Namespace.to_path("", namespace),
    full: namespace |> Namespace.to_path(root_dir),
  });

let _create_module_resolver = (root_dir, allowed, namespace) =>
  if (List.mem(namespace, allowed)) {
    _create_module(root_dir, namespace);
  } else {
    namespace
    |> Namespace.to_string
    |> Fmt.str("module %s not expected")
    |> Assert.fail;
  };

let _create_import_resolver = (~default=[], entries, id) =>
  entries |> List.assoc_opt(id) |?: default;

let suite =
  "Compile.Compiler"
  >::: [
    "create()"
    >: (
      () =>
        Assert.compiler(
          {
            config: Cx.config,
            graph: ImportGraph.create(_ => []),
            modules: ModuleTable.create(1),
            resolver: {
              cache: "",
              root_dir: Fx.valid_program_dir,
              source_dir: ".",
            },
            dispatch: _ => (),
          },
          Compiler.create(Cx.config),
        )
    ),
    "#dispatch() - test default error handler with fail_fast"
    >: (
      () => {
        let errors = [InvalidModule(Nx.foo)];
        let compiler = Compiler.create(Cx.config);

        Assert.throws_compile_errors(errors, () =>
          compiler.dispatch(Report(errors))
        );
      }
    ),
    "#dispatch() - test default error handler without fail_fast"
    >: (
      () => {
        let errors = [InvalidModule(Nx.foo)];
        let compiler = Compiler.create({...Cx.config, fail_fast: false});

        compiler.dispatch(Report(errors));

        Assert.throws_compile_errors(errors, () => compiler.dispatch(Flush));
      }
    ),
    "#dispatch() - test custom error handler"
    >: (
      () => {
        let errors = [InvalidModule(Nx.foo)];
        let compiler =
          Compiler.create(
            ~report=_ => Assert.compile_errors(errors),
            Cx.config,
          );

        compiler.dispatch(Report(errors));
      }
    ),
    "cache_modules()"
    >: (
      () => {
        let root_dir = Util.get_temp_dir();
        let cache = File.Cache.create("foo");
        let compiler = {
          ...Compiler.create(Cx.config),
          resolver: Resolve.Resolver.create(cache, root_dir, "."),
          graph: ImportGraph.create(_ => []),
        };

        FileUtil.cp(~recurse=true, [Fx.valid_program_dir], root_dir);

        compiler |> Compiler.cache_modules([Nx.entry, Nx.other]);

        __entry_filename |> Filename.concat(cache) |> Assert.file_exists;
        "other.kn" |> Filename.concat(cache) |> Assert.file_exists;
      }
    ),
    "validate() - catch cyclic imports"
    >: (
      () => {
        let compiler = {
          ...Compiler.create(Cx.config),
          graph: ImportGraph.create(_ => []),
        };

        compiler.graph.imports.nodes = [Nx.foo, Nx.bar];
        compiler.graph.imports.edges = [(Nx.foo, Nx.bar), (Nx.bar, Nx.foo)];

        Assert.throws_compile_errors([ImportCycle(["@/bar", "@/foo"])], () =>
          Compiler.validate(compiler)
        );
      }
    ),
    "validate() - catch unresolved module"
    >: (
      () => {
        let compiler = {
          ...Compiler.create(Cx.config),
          graph: ImportGraph.create(_ => []),
        };

        compiler.graph.imports.nodes = [Nx.foo];
        compiler.graph.imports.edges = [(Nx.foo, Nx.bar)];

        Assert.throws_compile_errors([UnresolvedModule("@/bar")], () =>
          Compiler.validate(compiler)
        );
      }
    ),
    "process_one() - parse a single module into its AST"
    >: (
      () => {
        let compiler = Compiler.create(Cx.config);

        compiler
        |> Compiler.process_one(
             Nx.entry,
             _create_module(Fx.valid_program_dir, Nx.entry),
           );

        Assert.module_table(
          [
            (
              Nx.entry,
              ModuleTable.Valid(
                "const ABC = 123;\n",
                {exports: __types, ast: Px.const_int, scopes: __scope_tree},
              ),
            ),
          ]
          |> List.to_seq
          |> Hashtbl.of_seq,
          compiler.modules,
        );
      }
    ),
    "process_one() - records errors captured during parsing"
    >: (
      () => {
        let expected = [
          ParseError(
            ReservedKeyword("const"),
            Nx.foo,
            Range.create((3, 7), (3, 11)),
          ),
        ];
        let compiler = Compiler.create({...Cx.config, fail_fast: false});

        compiler
        |> Compiler.process_one(
             ~flush=false,
             Nx.foo,
             _create_module(Fx.invalid_program_dir, Nx.foo),
           );

        Assert.module_table(
          [
            (
              Nx.foo,
              ModuleTable.Partial(
                "import { BAR } from \"@/bar\";

const const = \"foo\";
",
                {
                  exports:
                    [(Export.Named("const"), T.Valid(`String))]
                    |> List.to_seq
                    |> Hashtbl.of_seq,
                  ast: Px.invalid_foo,
                  scopes: __scope_tree,
                },
                expected,
              ),
            ),
          ]
          |> List.to_seq
          |> Hashtbl.of_seq,
          compiler.modules,
        );
        Assert.throws_compile_errors(expected, () =>
          compiler.dispatch(Flush)
        );
      }
    ),
    "process() - parse all modules into their AST"
    >: (
      () => {
        let compiler = Compiler.create(Cx.config);

        compiler
        |> Compiler.process(
             [Nx.entry, Nx.other],
             _create_module_resolver(
               Fx.valid_program_dir,
               [Nx.entry, Nx.other],
             ),
           );

        Assert.module_table(
          [
            (
              Nx.entry,
              ModuleTable.Valid(
                "const ABC = 123;\n",
                {exports: __types, ast: Px.const_int, scopes: __scope_tree},
              ),
            ),
            (
              Nx.other,
              ModuleTable.Valid(
                "import { ABC } from \"@/entry\";

const BAR = \"bar\";
",
                {
                  exports:
                    [(Export.Named("BAR"), T.Valid(`String))]
                    |> List.to_seq
                    |> Hashtbl.of_seq,
                  ast: Px.import_and_const,
                  scopes: __scope_tree,
                },
              ),
            ),
          ]
          |> List.to_seq
          |> Hashtbl.of_seq,
          compiler.modules,
        );
      }
    ),
    "process() - catch file not found"
    >: (
      () => {
        let compiler = Compiler.create(Cx.config);

        Assert.throws_compile_errors([FileNotFound(__entry_filename)], () =>
          compiler
          |> Compiler.process([Nx.entry], _ =>
               Source.File({relative: __entry_filename, full: "foo"})
             )
        );
      }
    ),
    "process() - catch parsing error"
    >: (
      () => {
        let compiler =
          Compiler.create({...Cx.config, root_dir: Fx.invalid_program_dir});

        Assert.throws_compile_errors([InvalidModule(Nx.entry)], () =>
          compiler
          |> Compiler.process(
               [Nx.entry],
               _create_module_resolver(Fx.invalid_program_dir, [Nx.entry]),
             )
        );
      }
    ),
    "init() - add modules to import graph"
    >: (
      () => {
        let compiler = Compiler.create(Cx.config);

        compiler |> Compiler.init(Nx.other);

        Assert.namespace_graph(
          Graph.create([Nx.entry, Nx.other], [(Nx.other, Nx.entry)]),
          compiler.graph.imports,
        );
      }
    ),
    "init() - add AST to module table"
    >: (
      () => {
        let compiler = Compiler.create(Cx.config);

        compiler |> Compiler.init(Nx.entry);

        Assert.module_table(
          [
            (
              Nx.entry,
              ModuleTable.Valid(
                "const ABC = 123;\n",
                {exports: __types, ast: Px.const_int, scopes: __scope_tree},
              ),
            ),
          ]
          |> List.to_seq
          |> Hashtbl.of_seq,
          compiler.modules,
        );
      }
    ),
    "init() - cache modules"
    >: (
      () => {
        let compiler = Compiler.create(Cx.config);

        compiler |> Compiler.init(Nx.entry);

        __entry_filename
        |> Filename.concat(compiler.resolver.cache)
        |> Assert.file_exists;
      }
    ),
    "init() - skip caching modules"
    >: (
      () => {
        let compiler = Compiler.create(Cx.config);

        compiler |> Compiler.init(~skip_cache=true, Nx.entry);

        __entry_filename
        |> Filename.concat(compiler.resolver.cache)
        |> Assert.no_file_exists;
      }
    ),
    "init() - validate imports"
    >: (
      () => {
        let compiler = Compiler.create(Cx.cyclic_config);

        Assert.throws_compile_errors(
          [ImportCycle(["@/entry", "@/cycle"])], () =>
          compiler |> Compiler.init(Nx.entry)
        );
      }
    ),
    "incremental() - add AST to module table"
    >: (
      () => {
        let compiler = Compiler.create(Cx.config);

        compiler |> Compiler.incremental([Nx.entry]);

        Assert.module_table(
          [
            (
              Nx.entry,
              ModuleTable.Valid(
                "const ABC = 123;\n",
                {exports: __types, ast: Px.const_int, scopes: __scope_tree},
              ),
            ),
          ]
          |> List.to_seq
          |> Hashtbl.of_seq,
          compiler.modules,
        );
      }
    ),
    "incremental() - validate imports"
    >: (
      () => {
        let compiler = {
          ...Compiler.create(Cx.cyclic_config),
          graph: {
            imports:
              Graph.create(
                [Nx.entry, Nx.foo],
                [(Nx.entry, Nx.foo), (Nx.foo, Nx.entry)],
              ),
            get_imports: _ => [],
          },
        };

        Assert.throws_compile_errors([ImportCycle(["@/foo", "@/entry"])], () =>
          compiler |> Compiler.incremental([Nx.entry])
        );
      }
    ),
    "emit_one() - write a compiled module"
    >: (
      () => {
        let output_dir = Util.get_temp_dir();
        let compiler = Compiler.create(Cx.config);

        Compiler.emit_one(
          Target.JavaScript(Common),
          output_dir,
          Nx.entry,
          compiler,
          ModuleTable.Valid(
            "",
            {
              ast: Px.import_and_const,
              exports: __types,
              scopes: __scope_tree,
            },
          ),
        );

        Assert.file_contents(
          Ox.import_and_const,
          Filename.concat(output_dir, "entry.js"),
        );
      }
    ),
    "emit() - write all modules from the table"
    >: (
      () => {
        let output_dir = Util.get_temp_dir();
        let compiler = {
          ...Compiler.create(Cx.config),
          modules:
            [
              (
                Nx.foo,
                ModuleTable.Valid(
                  "",
                  {
                    ast: Px.import_and_const,
                    exports: __types,
                    scopes: __scope_tree,
                  },
                ),
              ),
              (
                Nx.bar,
                ModuleTable.Valid(
                  "",
                  {
                    ast: Px.single_import,
                    exports: __types,
                    scopes: __scope_tree,
                  },
                ),
              ),
            ]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        compiler |> Compiler.emit(Target.JavaScript(Common), output_dir);

        Assert.file_contents(
          Ox.import_and_const,
          Filename.concat(output_dir, "foo.js"),
        );
        Assert.file_contents(
          Ox.single_import,
          Filename.concat(output_dir, "bar.js"),
        );
      }
    ),
    "compile() - generate import graph, process modules and output to target"
    >: (
      () => {
        let output_dir = Util.get_temp_dir();
        let compiler = Compiler.create(Cx.config);

        compiler
        |> Compiler.compile(Target.JavaScript(Common), output_dir, Nx.other);

        Assert.file_contents(
          Ox.const_int,
          Filename.concat(output_dir, "entry.js"),
        );
        Assert.file_contents(
          "var $knot = require(\"@knot/runtime\");
var $import$_$entry = require(\"./entry\");
var ABC = $import$_$entry.ABC;
$import$_$entry = null;
var BAR = \"bar\";
exports.BAR = BAR;
",
          Filename.concat(output_dir, "other.js"),
        );
      }
    ),
    "compile() - clear output directory"
    >: (
      () => {
        let output_dir = Util.get_temp_dir();
        let temp_file = "temp" |> Filename.concat(output_dir);
        let compiler = Compiler.create(Cx.config);

        FileUtil.mkdir(~parent=true, output_dir);
        FileUtil.touch(temp_file);

        compiler
        |> Compiler.compile(Target.JavaScript(Common), output_dir, Nx.other);

        Assert.no_file_exists(temp_file);
      }
    ),
    "add_module() - add new module and dependencies to import graph"
    >: (
      () => {
        let compiler = Compiler.create(Cx.config);

        let updated = compiler |> Compiler.add_module(Nx.other);

        Assert.list_namespace([Nx.entry, Nx.other], updated);
        Assert.module_table(
          [(Nx.entry, ModuleTable.Pending), (Nx.other, ModuleTable.Pending)]
          |> List.to_seq
          |> Hashtbl.of_seq,
          compiler.modules,
        );
      }
    ),
    "add_module() - include dependents in updated modules"
    >: (
      () => {
        let compiler = {
          ...Compiler.create(Cx.config),
          graph: {
            imports:
              Graph.create(
                [Nx.foo, Nx.other],
                [(Nx.other, Nx.foo), (Nx.foo, Nx.bar)],
              ),
            get_imports: _create_import_resolver([(Nx.bar, [Nx.entry])]),
          },
        };

        let updated = compiler |> Compiler.add_module(Nx.bar);

        Assert.list_namespace([Nx.entry, Nx.bar, Nx.foo, Nx.other], updated);
      }
    ),
    "update_module() - update an existing module"
    >: (
      () => {
        let compiler = {
          ...Compiler.create(Cx.config),
          graph: {
            imports:
              Graph.create(
                [Nx.foo, Nx.bar, Nx.other],
                [(Nx.foo, Nx.bar), (Nx.bar, Nx.other)],
              ),
            get_imports: _create_import_resolver([(Nx.bar, [Nx.entry])]),
          },
        };

        let updated = compiler |> Compiler.update_module(Nx.bar);

        Assert.list_namespace([Nx.entry, Nx.bar, Nx.foo], updated);
        Assert.module_table(
          [
            (Nx.foo, ModuleTable.Pending),
            (Nx.bar, ModuleTable.Pending),
            (Nx.entry, ModuleTable.Pending),
          ]
          |> List.to_seq
          |> Hashtbl.of_seq,
          compiler.modules,
        );
      }
    ),
    "remove_module() - remove an existing module"
    >: (
      () => {
        let compiler = {
          ...Compiler.create(Cx.config),
          graph: {
            imports:
              Graph.create(
                [Nx.foo, Nx.bar, Nx.other],
                [(Nx.foo, Nx.bar), (Nx.bar, Nx.other)],
              ),
            get_imports: _create_import_resolver([(Nx.bar, [Nx.entry])]),
          },
        };

        compiler |> Compiler.remove_module(Nx.bar) |> ignore;

        Assert.module_table(
          [(Nx.foo, ModuleTable.Pending), (Nx.bar, ModuleTable.Purged)]
          |> List.to_seq
          |> Hashtbl.of_seq,
          compiler.modules,
        );
      }
    ),
    "inject_raw() - compile a raw module string and all related modules"
    >: (
      () => {
        let compiler = {
          ...Compiler.create(Cx.config),
          graph: {
            imports:
              Graph.create(
                [Nx.foo, Nx.other],
                [(Nx.foo, Nx.bar), (Nx.bar, Nx.other)],
              ),
            get_imports: _ => [],
          },
        };

        compiler
        |> Compiler.inject_raw(
             Nx.bar,
             "import { ABC } from \"@/entry\";
const XYZ = true;
",
           );

        Assert.namespace_graph(
          Graph.create(
            [Nx.entry, Nx.bar, Nx.foo, Nx.other],
            [(Nx.bar, Nx.entry), (Nx.foo, Nx.bar), (Nx.bar, Nx.other)],
          ),
          compiler.graph.imports,
        );
      }
    ),
    "reset() - reset compiler state"
    >: (
      () => {
        let compiler = {
          ...Compiler.create(Cx.config),
          graph: {
            imports:
              Graph.create(
                [Nx.foo, Nx.other],
                [(Nx.foo, Nx.bar), (Nx.bar, Nx.other)],
              ),
            get_imports: _ => [],
          },
        };
        let temp_file = "temp" |> Filename.concat(compiler.resolver.cache);

        FileUtil.mkdir(~parent=true, compiler.resolver.cache);
        FileUtil.touch(temp_file);

        Compiler.reset(compiler);

        Assert.namespace_graph(Graph.empty(), compiler.graph.imports);
        Assert.module_table(ModuleTable.create(0), compiler.modules);
        Assert.no_file_exists(temp_file);
      }
    ),
    "reset() - teardown compiler"
    >: (
      () => {
        let compiler = {
          ...Compiler.create(Cx.config),
          graph: {
            imports:
              Graph.create(
                [Nx.foo, Nx.other],
                [(Nx.foo, Nx.bar), (Nx.bar, Nx.other)],
              ),
            get_imports: _ => [],
          },
        };

        Compiler.teardown(compiler);

        Assert.namespace_graph(Graph.empty(), compiler.graph.imports);
        Assert.module_table(ModuleTable.create(0), compiler.modules);
        Assert.no_file_exists(compiler.resolver.cache);
      }
    ),
  ];
