open Kore;
open Reference;

module Compiler = Compile.Compiler;
module ImportGraph = Resolve.ImportGraph;
module Module = Resolve.Module;
module U = Util.ResultUtil;
module A = AST;
module T = Type;

let _fixture = Filename.concat("./test/compile/.fixtures");

let __valid_program_dir = _fixture("valid_program");
let __invalid_program_dir = _fixture("invalid_program");
let __cyclic_imports_dir = _fixture("cyclic_imports");
let __source_dir = ".";
let __entry_module = "entry";
let __entry_filename = "entry.kn";
let __entry = Namespace.Internal(__entry_module);
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

let __ast = [
  (
    ("ABC" |> A.of_public, Range.create((1, 7), (1, 9)))
    |> A.of_named_export,
    123L
    |> A.of_int
    |> A.of_num
    |> A.of_prim
    |> U.as_node(
         ~range=Range.create((1, 13), (1, 15)),
         T.Valid(`Integer),
       )
    |> A.of_const
    |> U.as_node(~range=Range.create((1, 13), (1, 15)), T.Valid(`Integer)),
  )
  |> A.of_decl
  |> U.as_raw_node(~range=Range.create((1, 1), (1, 15))),
];

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
        let foo = Namespace.Internal("foo");
        let bar = Namespace.Internal("bar");
        let compiler = {
          ...Compiler.create(__config),
          graph: ImportGraph.create(_ => []),
        };

        compiler.graph.imports.nodes = [foo, bar];
        compiler.graph.imports.edges = [(foo, bar), (bar, foo)];

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
        let foo = Namespace.Internal("foo");
        let bar = Namespace.Internal("bar");
        let compiler = {
          ...Compiler.create(__config),
          graph: ImportGraph.create(_ => []),
        };

        compiler.graph.imports.nodes = [foo];
        compiler.graph.imports.edges = [(foo, bar)];

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
             __entry,
             Module.File({
               relative: __entry_filename,
               full: Filename.concat(__valid_program_dir, __entry_filename),
             }),
           );

        Assert.module_table(
          [
            (
              __entry,
              ModuleTable.Valid({
                exports: __types,
                ast: __ast,
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
        let other = Namespace.Internal("other");
        let compiler = Compiler.create(__config);

        compiler
        |> Compiler.process(
             [__entry, other],
             fun
             | ns when ns == __entry =>
               Module.File({
                 relative: __entry_filename,
                 full: Filename.concat(__valid_program_dir, __entry_filename),
               })
             | ns when ns == other =>
               Module.File({
                 relative: "other.kn",
                 full: Filename.concat(__valid_program_dir, "other.kn"),
               })
             | ns =>
               ns
               |> Namespace.to_string
               |> Fmt.str("module %s not expected")
               |> Assert.fail,
           );

        Assert.module_table(
          [
            (
              __entry,
              ModuleTable.Valid({
                exports: __types,
                ast: __ast,
                scopes: __scope_tree,
                raw: "const ABC = 123;\n",
              }),
            ),
            (
              other,
              ModuleTable.Valid({
                exports:
                  [(Export.Named("BAR" |> A.of_public), T.Valid(`String))]
                  |> List.to_seq
                  |> Hashtbl.of_seq,
                ast: [
                  (
                    __entry,
                    [
                      (
                        "ABC"
                        |> A.of_public
                        |> U.as_raw_node(
                             ~range=Range.create((1, 10), (1, 12)),
                           ),
                        None,
                      )
                      |> A.of_named_import
                      |> U.as_raw_node(
                           ~range=Range.create((1, 10), (1, 12)),
                         ),
                    ],
                  )
                  |> A.of_import
                  |> U.as_raw_node(~range=Range.create((1, 1), (1, 29))),
                  (
                    ("BAR" |> A.of_public, Range.create((3, 7), (3, 9)))
                    |> A.of_named_export,
                    "bar"
                    |> A.of_string
                    |> A.of_prim
                    |> U.as_node(
                         ~range=Range.create((3, 13), (3, 17)),
                         T.Valid(`String),
                       )
                    |> A.of_const
                    |> U.as_node(
                         ~range=Range.create((3, 13), (3, 17)),
                         T.Valid(`String),
                       ),
                  )
                  |> A.of_decl
                  |> U.as_raw_node(~range=Range.create((3, 1), (3, 17))),
                ],
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
          |> Compiler.process([__entry], _ =>
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
          CompileError([InvalidModule(__entry)]),
          "should throw InvalidModule exception",
          () =>
          compiler
          |> Compiler.process([__entry], _ =>
               Module.File({
                 relative: __entry_filename,
                 full:
                   Filename.concat(__invalid_program_dir, __entry_filename),
               })
             )
        );
      }
    ),
    "init() - process project source files"
    >: (
      () => {
        let compiler = Compiler.create(__config);

        compiler |> Compiler.init(__entry);

        _assert_import_graph_structure(
          {imports: Graph.create([__entry], []), get_imports: _ => []},
          compiler.graph,
        );
        Assert.module_table(
          [
            (
              __entry,
              ModuleTable.Valid({
                exports: __types,
                ast: __ast,
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
          compiler |> Compiler.init(__entry)
        );
      }
    ),
  ];
