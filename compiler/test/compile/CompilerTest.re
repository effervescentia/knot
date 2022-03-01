open Kore;
open Reference;

module Compiler = Compile.Compiler;
module ImportGraph = Resolve.ImportGraph;
module Module = Resolve.Module;
module U = Util.ResultUtil;
module A = AST;
module T = TypeV2;

let __valid_program_dir = "./test/compile/.fixtures/valid_program";
let __invalid_program_dir = "./test/compile/.fixtures/invalid_program";
let __cyclic_imports_dir = "./test/compile/.fixtures/cyclic_imports";
let __source_dir = ".";
let __entry_module = "entry";
let __entry_filename = "entry.kn";
let __entry = Namespace.Internal(__entry_module);
let __config =
  Compiler.{
    name: "foo",
    root_dir: __valid_program_dir,
    source_dir: __source_dir,
    fail_fast: true,
  };

let __scope_tree = BinaryTree.create((Range.zero, None));

let __types =
  [(Export.Named("ABC" |> A.of_public), T.Valid(`Integer))]
  |> List.to_seq
  |> Hashtbl.of_seq;

let __ast =
  [
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
      |> U.as_node(
           ~range=Range.create((1, 13), (1, 15)),
           T.Valid(`Integer),
         ),
    )
    |> A.of_decl,
  ]
  |> List.map(U.as_raw_node);

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
    "process() - parse all modules into their AST"
    >: (
      () => {
        let compiler = Compiler.create(__config);

        compiler
        |> Compiler.process(
             [__entry],
             id => {
               Assert.namespace(__entry, id);
               Module.File({
                 relative: __entry_filename,
                 full: Filename.concat(__valid_program_dir, __entry_filename),
               });
             },
           );

        Assert.module_table(
          [
            (
              __entry,
              ModuleTable.{
                exports: __types,
                ast: __ast,
                scopes: __scope_tree,
                raw: "const ABC = 123;\n",
              },
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
    "init() - parse single module into AST"
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
              ModuleTable.{
                exports: __types,
                ast: __ast,
                scopes: __scope_tree,
                raw: "const ABC = 123;\n",
              },
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
          Compiler.create({...__config, root_dir: __cyclic_imports_dir});

        Assert.throws(
          CompileError([ImportCycle(["@/entry", "@/cycle"])]),
          "should throw ImportCycle exception",
          () =>
          compiler |> Compiler.init(__entry)
        );
      }
    ),
  ];
