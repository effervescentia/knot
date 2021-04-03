open Kore;
open Util;
open Type;

module Compiler = Compile.Compiler;
module ImportGraph = Resolve.ImportGraph;
module Module = Resolve.Module;

let __valid_program_dir = "./test/compile/.fixtures/valid_program";
let __invalid_program_dir = "./test/compile/.fixtures/invalid_program";
let __cyclic_imports_dir = "./test/compile/.fixtures/cyclic_imports";
let __source_dir = ".";
let __entry_module = "entry";
let __entry_filename = "entry.kn";
let __entry = Reference.Namespace.Internal(__entry_module);
let __config =
  Compiler.{
    name: "foo",
    root_dir: __valid_program_dir,
    source_dir: __source_dir,
    fail_fast: true,
  };

let __types =
  AST.[("ABC" |> of_public, K_Strong(K_Integer))]
  |> List.to_seq
  |> Hashtbl.of_seq;
let __ast =
  AST.[
    (
      ("ABC" |> of_public, Cursor.range((1, 7), (1, 9))),
      (
        123 |> Int64.of_int |> of_int |> of_num,
        K_Strong(K_Integer),
        Cursor.range((1, 13), (1, 15)),
      )
      |> of_prim
      |> as_typed_lexeme(
           ~cursor=Cursor.range((1, 13), (1, 15)),
           K_Strong(K_Integer),
         )
      |> of_const,
    )
    |> of_decl,
  ];

let _assert_import_graph_structure =
  Alcotest.(
    check(
      testable(
        pp => Resolve.Debug.print_import_graph % Format.pp_print_string(pp),
        (l, r) => l.imports == r.imports,
      ),
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
          [(__entry, ModuleTable.{types: __types, ast: __ast})]
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

        Alcotest.check_raises(
          "should throw FileNotFound exception",
          CompileError([FileNotFound(__entry_filename)]),
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

        Alcotest.check_raises(
          "should throw ParseFailed exception", ParseFailed, () =>
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
          [(__entry, ModuleTable.{types: __types, ast: __ast})]
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

        Alcotest.check_raises(
          "should throw ImportCycle exception",
          CompileError([ImportCycle(["@/entry", "@/cycle"])]),
          () =>
          compiler |> Compiler.init(__entry)
        );
      }
    ),
  ];
