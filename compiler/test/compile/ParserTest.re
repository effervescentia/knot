open Kore;
open Util;

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
            _to_stream(__import_fixture) |> Parser.imports,
          ),
          ([], _to_stream("") |> Parser.imports),
        ]
        |> Assert.(test_many(list_namespace))
    ),
    "parse AST"
    >: (
      () =>
        [
          ([], _to_stream("") |> Parser.ast),
          (
            AST.[
              of_import(("bar" |> of_internal, "foo")),
              of_decl((
                ("ABC" |> of_public, Cursor.range((4, 9), (4, 11))),
                (
                  123 |> Int64.of_int |> of_int |> of_num,
                  Type.K_Integer,
                  Cursor.range((4, 15), (4, 17)),
                )
                |> of_prim
                |> as_typed_lexeme(
                     ~cursor=Cursor.range((4, 15), (4, 17)),
                     Type.K_Integer,
                   )
                |> of_const,
              )),
            ],
            _to_stream(__ast_fixture) |> Parser.ast,
          ),
        ]
        |> Assert.(test_many(program))
    ),
    "parse invalid"
    >: (
      () =>
        Alcotest.check_raises("should throw ParseFailed", ParseFailed, () =>
          _to_stream("foo bar") |> Parser.ast |> ignore
        )
    ),
  ];
