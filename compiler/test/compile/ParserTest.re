open Kore;

module Namespace = Reference.Namespace;
module Export = Reference.Export;
module Parser = Compile.Parser;
module U = Util.ResultUtil;
module A = AST.Result;
module ModuleTable = AST.ModuleTable;
module ParseContext = AST.ParseContext;
module SymbolTable = AST.SymbolTable;
module Type = AST.Type;
module Nx = Fixtures.Namespace;
module Rx = Fixtures.Raw;
module Px = Fixtures.Program;

let __scope_tree = BinaryTree.create((Range.zero, None));

let _create_module =
    (exports: list((Export.t, Type.t))): ModuleTable.module_t => {
  ast: [],
  scopes: __scope_tree,
  symbols: SymbolTable.of_export_list(exports),
};

let _create_module_table = modules =>
  ModuleTable.{
    modules: modules |> List.to_seq |> Hashtbl.of_seq,
    plugins: [],
    globals: [],
  };

let __context =
  ParseContext.create(
    ~modules=
      [
        (
          Nx.bar,
          ModuleTable.Valid(
            "foo",
            _create_module([(Export.Main, Type.Valid(`Boolean))]),
          ),
        ),
      ]
      |> _create_module_table,
    Nx.foo,
  );

let _to_stream = string =>
  File.InputStream.of_string(string) |> LazyStream.of_stream;

let suite =
  "Compile.Parser"
  >::: [
    "parse imports - empty file"
    >: (
      () =>
        Assert.list_namespace([], "" |> _to_stream |> Parser.imports(Nx.foo))
    ),
    "parse imports - module with imports"
    >: (
      () =>
        Assert.list_namespace(
          [A.of_external("bar"), A.of_external("buzz")],
          Rx.multiple_import |> _to_stream |> Parser.imports(Nx.foo),
        )
    ),
    "parse AST - empty file"
    >: (
      () =>
        Assert.program(
          [],
          "" |> _to_stream |> Parser.ast(__context) |> Result.get_ok,
        )
    ),
    "parse AST - module with declarations and imports"
    >: (
      () =>
        Assert.program(
          Px.single_import,
          Rx.single_import
          |> _to_stream
          |> Parser.ast(__context)
          |> Result.get_ok,
        )
    ),
    "parse invalid"
    >: (
      () =>
        Assert.result_program(
          Error(InvalidModule(Nx.foo)),
          "foo bar" |> _to_stream |> Parser.ast(__context),
        )
    ),
  ];
