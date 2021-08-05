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

let __scope_tree = BinaryTree.create((Cursor.zero |> Cursor.expand, None));

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
    "parse imports"
    >: (
      () =>
        AST.[
          (
            [of_external("bar"), of_external("buzz")],
            _to_stream(__import_fixture) |> Parser.imports(__foo),
          ),
          ([], _to_stream("") |> Parser.imports(__foo)),
        ]
        |> Assert.(test_many(list_namespace))
    ),
    "parse AST"
    >: (
      () =>
        [
          ([], _to_stream("") |> Parser.ast(__context)),
          (
            AST.[
              (
                __bar,
                [
                  "foo"
                  |> of_public
                  |> as_raw_node(~cursor=Cursor.range((2, 10), (2, 12)))
                  |> of_main_import
                  |> as_raw_node,
                ],
              )
              |> of_import,
              (
                ("ABC" |> of_public, Cursor.range((4, 9), (4, 11)))
                |> of_named_export,
                (
                  123 |> Int64.of_int |> of_int |> of_num,
                  Type.Valid(`Integer),
                  Cursor.range((4, 15), (4, 17)),
                )
                |> of_prim
                |> as_node(
                     ~cursor=Cursor.range((4, 15), (4, 17)),
                     Type.Valid(`Integer),
                   )
                |> of_const
                |> as_node(
                     ~cursor=Cursor.range((4, 15), (4, 17)),
                     Type.Valid(`Integer),
                   ),
              )
              |> of_decl,
            ]
            |> List.map(as_raw_node),
            _to_stream(__ast_fixture) |> Parser.ast(__context),
          ),
        ]
        |> Assert.(test_many(program))
    ),
    "parse invalid"
    >: (
      () =>
        Alcotest.check_raises(
          "should throw InvalidModule",
          CompileError([InvalidModule(__foo)]),
          () =>
          _to_stream("foo bar") |> Parser.ast(__context) |> ignore
        )
    ),
  ];
