open Kore;

module Compiler = Compile.Compiler;
module ModuleTable = Compile.ModuleTable;
module ImportGraph = Resolve.ImportGraph;
module Module = Resolve.Module;

let __valid_program_dir = "./test/compile/.fixtures/valid_program";
let __invalid_program_dir = "./test/compile/.fixtures/invalid_program";
let __cyclic_imports_dir = "./test/compile/.fixtures/cyclic_imports";
let __source_dir = ".";
let __entry_module = "entry";
let __entry_filename = "entry.kn";
let __entry = Internal(__entry_module);
let __config =
  Compiler.{
    name: "foo",
    entry: __entry,
    root_dir: __valid_program_dir,
    source_dir: __source_dir,
  };

let __types = [("ABC", Type.K_Invalid)] |> List.to_seq |> Hashtbl.of_seq;
let __ast =
  AST.[
    of_decl((
      "ABC",
      of_const(
        of_prim(
          Block.create(
            ~type_=Type.K_Integer,
            Cursor.range((1, 13), (1, 15)),
            of_num(
              Block.create(
                ~type_=Type.K_Integer,
                Cursor.range((1, 13), (1, 15)),
                of_int(Int64.of_int(123)),
              ),
            ),
          ),
        ),
      ),
    )),
  ];

let _assert_import_graph_structure =
  Alcotest.(
    check(
      testable(
        pp => Resolve.ImportGraph.to_string % Format.pp_print_string(pp),
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
            errors: ref([]),
            throw: throw_all,
          },
          Compiler.create(__config),
        )
    ),
    "create() - use custom error handler"
    >: (
      () => {
        let throw = _ => ();

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
            errors: ref([]),
            throw,
          },
          Compiler.create(~catch=throw, __config),
        );
      }
    ),
    "process() - parse all modules into their AST"
    >: (
      () => {
        let compiler = Compiler.create(__config);

        compiler
        |> Compiler.process(
             [__entry],
             id => {
               Assert.m_id(__entry, id);
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
          CompilerError([FileNotFound(__entry_filename)]),
          () =>
          compiler
          |> Compiler.process([__entry], _ =>
               Module.File({relative: __entry_filename, full: "foo"})
             )
        );
        Assert.int(0, List.length(compiler.errors^));
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

        compiler |> Compiler.init;

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
          CompilerError([ImportCycle(["@/entry", "@/cycle"])]),
          () =>
          compiler |> Compiler.init
        );
      }
    ),
  ];
