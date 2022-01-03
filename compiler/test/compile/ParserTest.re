open Kore;
open Util.ResultUtil;
open Reference;

module Parser = Compile.Parser;

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
      AST.[
        (
          "bar" |> of_internal,
          ModuleTable.{
            ast: [],
            exports:
              [(Export.Main, Type.Valid(`Boolean))]
              |> List.to_seq
              |> Hashtbl.of_seq,
            scopes: __scope_tree,
            raw: "foo",
          },
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
          AST.[of_external("bar"), of_external("buzz")],
          _to_stream(__import_fixture) |> Parser.imports(__foo),
        )
    ),
    "parse AST - empty file"
    >: (() => Assert.program([], _to_stream("") |> Parser.ast(__context))),
    "parse AST - module with declarations and imports"
    >: (
      () =>
        Assert.program(
          AST.[
            (
              __bar,
              [
                "foo"
                |> of_public
                |> as_raw_node(~range=Range.create((2, 10), (2, 12)))
                |> of_main_import
                |> as_raw_node(~range=Range.create((2, 10), (2, 12))),
              ],
            )
            |> of_import
            |> as_raw_node(~range=Range.create((2, 3), (2, 25))),
            (
              ("ABC" |> of_public, Range.create((4, 9), (4, 11)))
              |> of_named_export,
              (
                123L |> of_int |> of_num,
                Type.Valid(`Integer),
                Range.create((4, 15), (4, 17)),
              )
              |> of_prim
              |> as_node(
                   ~range=Range.create((4, 15), (4, 17)),
                   Type.Valid(`Integer),
                 )
              |> of_const
              |> as_node(
                   ~range=Range.create((4, 15), (4, 17)),
                   Type.Valid(`Integer),
                 ),
            )
            |> of_decl
            |> as_raw_node(~range=Range.create((4, 3), (4, 17))),
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
