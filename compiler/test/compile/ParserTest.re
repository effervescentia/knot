open Kore;

module Namespace = Reference.Namespace;
module Export = Reference.Export;
module Parser = Compile.Parser;
module U = Util.ResultUtil;
module A = AST;

let __import_fixture = "
  import foo from \"bar\";
  import fizz from \"buzz\";

  const ABC = 123;
 ";
let __ast_fixture = "
  import foo from \"@/bar\";

  const ABC = 123;
 ";
let __foo = Namespace.Internal("foo");
let __bar = Namespace.Internal("bar");

let __scope_tree = BinaryTree.create((Range.zero, None));

let __context =
  NamespaceContext.create(
    ~modules=
      [
        (
          "bar" |> A.of_internal,
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
    __foo,
  );

let _to_stream = string =>
  File.InputStream.of_string(string) |> LazyStream.of_stream;

let suite =
  "Compile.Parser"
  >::: [
    "parse imports - empty file"
    >: (
      () =>
        Assert.list_namespace([], _to_stream("") |> Parser.imports(__foo))
    ),
    "parse imports - module with imports"
    >: (
      () =>
        Assert.list_namespace(
          [A.of_external("bar"), A.of_external("buzz")],
          _to_stream(__import_fixture) |> Parser.imports(__foo),
        )
    ),
    "parse AST - empty file"
    >: (() => Assert.program([], _to_stream("") |> Parser.ast(__context))),
    "parse AST - module with declarations and imports"
    >: (
      () =>
        Assert.program(
          [
            (
              __bar,
              [
                "foo"
                |> A.of_public
                |> U.as_raw_node(~range=Range.create((2, 10), (2, 12)))
                |> A.of_main_import
                |> U.as_raw_node(~range=Range.create((2, 10), (2, 12))),
              ],
            )
            |> A.of_import
            |> U.as_raw_node(~range=Range.create((2, 3), (2, 25))),
            (
              ("ABC" |> A.of_public, Range.create((4, 9), (4, 11)))
              |> A.of_named_export,
              123L
              |> A.of_int
              |> A.of_num
              |> A.of_prim
              |> U.as_node(
                   ~range=Range.create((4, 15), (4, 17)),
                   Type.Valid(`Integer),
                 )
              |> A.of_const
              |> U.as_node(
                   ~range=Range.create((4, 15), (4, 17)),
                   Type.Valid(`Integer),
                 ),
            )
            |> A.of_decl
            |> U.as_raw_node(~range=Range.create((4, 3), (4, 17))),
          ],
          _to_stream(__ast_fixture) |> Parser.ast(__context),
        )
    ),
    "parse invalid"
    >: (
      () =>
        Assert.throws(
          CompileError([InvalidModule(__foo)]),
          "should throw InvalidModule",
          () =>
          _to_stream("foo bar") |> Parser.ast(__context) |> ignore
        )
    ),
  ];
