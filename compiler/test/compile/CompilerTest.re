open Kore;
open Reference;

module Compiler = Compile.Compiler;
module ImportGraph = Resolve.ImportGraph;
module Module = Resolve.Module;
module U = Util.ResultUtil;
module A = AST;
module T = Type;
module P = Fixtures.Program;
module N = Fixtures.Namespace;

let _fixture = Filename.concat("./test/compile/.fixtures");

let __valid_program_dir = _fixture("valid_program");
let __invalid_program_dir = _fixture("invalid_program");
let __cyclic_imports_dir = _fixture("cyclic_imports");
let __source_dir = ".";
let __entry_filename = "entry.kn";
let __config =
  Compiler.{
    name: "foo",
    root_dir: __valid_program_dir,
    source_dir: ".",
    fail_fast: true,
    log_imports: false,
  };

let __scope_tree = BinaryTree.create((Range.zero, None));

let __types =
  [(Export.Named("ABC" |> A.of_public), T.Valid(`Integer))]
  |> List.to_seq
  |> Hashtbl.of_seq;

let _create_module = (root_dir, namespace) =>
  Module.File({
    relative: Namespace.to_path("", namespace),
    full: namespace |> Namespace.to_path(root_dir),
  });

let _create_resolver = (root_dir, allowed, namespace) =>
  if (List.mem(namespace, allowed)) {
    _create_module(root_dir, namespace);
  } else {
    namespace
    |> Namespace.to_string
    |> Fmt.str("module %s not expected")
    |> Assert.fail;
  };

let _assert_import_graph_structure =
  Alcotest.(
    check(
      testable(ImportGraph.pp, (l, r) => l.imports == r.imports),
      "import graph matches",
    )
  );

let suite =
  "Compile.Compiler"
  >::: [
    "create() - use global error handler"
    >: (
      () =>
        Assert.compiler(
          {
            config: __config,
            graph: ImportGraph.create(_ => []),
            modules: ModuleTable.create(1),
            resolver: {
              cache: "",
              root_dir: __valid_program_dir,
              source_dir: __source_dir,
            },
            dispatch: _ => (),
          },
          Compiler.create(__config),
        )
    ),
    "create() - use custom error handler"
    >: (
      () =>
        Assert.compiler(
          {
            config: __config,
            graph: ImportGraph.create(_ => []),
            modules: ModuleTable.create(1),
            resolver: {
              cache: "",
              root_dir: __valid_program_dir,
              source_dir: __source_dir,
            },
            dispatch: _ => (),
          },
          Compiler.create(__config),
        )
    ),
    "validate() - catch cyclic imports"
    >: (
      () => {
        let compiler = {
          ...Compiler.create(__config),
          graph: ImportGraph.create(_ => []),
        };

        compiler.graph.imports.nodes = [N.foo, N.bar];
        compiler.graph.imports.edges = [(N.foo, N.bar), (N.bar, N.foo)];

        Assert.throws(
          CompileError([ImportCycle(["@/bar", "@/foo"])]),
          "should throw ImportCycle exception",
          () =>
          Compiler.validate(compiler)
        );
      }
    ),
    "validate() - catch unresolved module"
    >: (
      () => {
        let compiler = {
          ...Compiler.create(__config),
          graph: ImportGraph.create(_ => []),
        };

        compiler.graph.imports.nodes = [N.foo];
        compiler.graph.imports.edges = [(N.foo, N.bar)];

        Assert.throws(
          CompileError([UnresolvedModule("@/bar")]),
          "should throw UnresolvedModule exception",
          () =>
          Compiler.validate(compiler)
        );
      }
    ),
    "process_one() - parse a single module into its AST"
    >: (
      () => {
        let compiler = Compiler.create(__config);

        compiler
        |> Compiler.process_one(
             N.entry,
             _create_module(__valid_program_dir, N.entry),
           );

        Assert.module_table(
          [
            (
              N.entry,
              ModuleTable.Valid({
                exports: __types,
                ast: P.const_int,
                scopes: __scope_tree,
                raw: "const ABC = 123;\n",
              }),
            ),
          ]
          |> List.to_seq
          |> Hashtbl.of_seq,
          compiler.modules,
        );
      }
    ),
    "process() - parse all modules into their AST"
    >: (
      () => {
        let compiler = Compiler.create(__config);

        compiler
        |> Compiler.process(
             [N.entry, N.other],
             _create_resolver(__valid_program_dir, [N.entry, N.other]),
           );

        Assert.module_table(
          [
            (
              N.entry,
              ModuleTable.Valid({
                exports: __types,
                ast: P.const_int,
                scopes: __scope_tree,
                raw: "const ABC = 123;\n",
              }),
            ),
            (
              N.other,
              ModuleTable.Valid({
                exports:
                  [(Export.Named("BAR" |> A.of_public), T.Valid(`String))]
                  |> List.to_seq
                  |> Hashtbl.of_seq,
                ast: P.import_and_const,
                scopes: __scope_tree,
                raw: "import { ABC } from \"@/entry\";

const BAR = \"bar\";
",
              }),
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
        let compiler = Compiler.create(__config);

        Assert.throws(
          CompileError([FileNotFound(__entry_filename)]),
          "should throw FileNotFound exception",
          () =>
          compiler
          |> Compiler.process([N.entry], _ =>
               Module.File({relative: __entry_filename, full: "foo"})
             )
        );
      }
    ),
    "process() - catch parsing error"
    >: (
      () => {
        let compiler =
          Compiler.create({...__config, root_dir: __invalid_program_dir});

        Assert.throws(
          CompileError([InvalidModule(N.entry)]),
          "should throw InvalidModule exception",
          () =>
          compiler
          |> Compiler.process(
               [N.entry],
               _create_resolver(__invalid_program_dir, [N.entry]),
             )
        );
      }
    ),
    "init() - process project source files"
    >: (
      () => {
        let compiler = Compiler.create(__config);

        compiler |> Compiler.init(N.entry);

        _assert_import_graph_structure(
          {imports: Graph.create([N.entry], []), get_imports: _ => []},
          compiler.graph,
        );
        Assert.module_table(
          [
            (
              N.entry,
              ModuleTable.Valid({
                exports: __types,
                ast: P.const_int,
                scopes: __scope_tree,
                raw: "const ABC = 123;\n",
              }),
            ),
          ]
          |> List.to_seq
          |> Hashtbl.of_seq,
          compiler.modules,
        );
      }
    ),
    "init() - catch cyclic imports"
    >: (
      () => {
        let compiler =
          Compiler.create({
            ...__config,
            root_dir: __cyclic_imports_dir,
            source_dir: ".",
          });

        Assert.throws(
          CompileError([ImportCycle(["@/entry", "@/cycle"])]),
          "should throw ImportCycle exception",
          () =>
          compiler |> Compiler.init(N.entry)
        );
      }
    ),
  ];
