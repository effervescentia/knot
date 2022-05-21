open Kore;

module Namespace = Reference.Namespace;
module Export = Reference.Export;
module Parser = Compile.Parser;
module U = Util.ResultUtil;
module A = AST;
module N = Fixtures.Namespace;
module R = Fixtures.Raw;
module P = Fixtures.Program;

let __scope_tree = BinaryTree.create((Range.zero, None));

let __context =
  NamespaceContext.create(
    ~modules=
      [
        (
          N.bar,
          ModuleTable.Valid({
            ast: [],
            exports:
              [(Export.Main, Type.Valid(`Boolean))]
              |> List.to_seq
              |> Hashtbl.of_seq,
            scopes: __scope_tree,
            raw: "foo",
          }),
        ),
      ]
      |> List.to_seq
      |> Hashtbl.of_seq,
    N.foo,
  );

let _to_stream = string =>
  File.InputStream.of_string(string) |> LazyStream.of_stream;

let suite =
  "Compile.Parser"
  >::: [
    "parse imports - empty file"
    >: (
      () =>
        Assert.list_namespace([], "" |> _to_stream |> Parser.imports(N.foo))
    ),
    "parse imports - module with imports"
    >: (
      () =>
        Assert.list_namespace(
          [A.of_external("bar"), A.of_external("buzz")],
          R.multiple_import |> _to_stream |> Parser.imports(N.foo),
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
          P.single_import,
          R.single_import
          |> _to_stream
          |> Parser.ast(__context)
          |> Result.get_ok,
        )
    ),
    "parse invalid"
    >: (
      () =>
        Assert.result_program(
          Error(InvalidModule(N.foo)),
          "foo bar" |> _to_stream |> Parser.ast(__context),
        )
    ),
  ];
