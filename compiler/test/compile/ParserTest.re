open Kore;

module Parser = Compile.Parser;

let __import_fixture = "
  import foo from \"bar\";
  import fizz from \"buzz\";

  const ABC = 123;
 ";
let __ast_fixture = "
  import foo from \"bar\";

  const ABC = 123;
 ";

let _range = ((l, l'), (r, r')) =>
  Cursor.range(Cursor.point(l, l'), Cursor.point(r, r'));
let _to_stream = string =>
  File.CharStream.of_string(string) |> LazyStream.of_stream;

let suite =
  "Compile.Parser"
  >::: [
    "parse imports"
    >: (
      () =>
        [
          (
            [External("bar.kn"), External("buzz.kn")],
            _to_stream(__import_fixture) |> Parser.imports,
          ),
          ([], _to_stream("") |> Parser.imports),
        ]
        |> Assert.(test_many(list_m_id))
    ),
    "parse AST"
    >: (
      () =>
        [
          ([], _to_stream("") |> Parser.ast),
          (
            [
              AST.of_import(("bar", "foo")),
              AST.of_decl((
                "ABC",
                AST.of_const(
                  AST.of_prim(
                    Block.create(
                      ~type_=Type.K_Integer,
                      _range((4, 15), (4, 17)),
                      AST.of_num(
                        Block.create(
                          ~type_=Type.K_Integer,
                          _range((4, 15), (4, 17)),
                          AST.of_int(Int64.of_int(123)),
                        ),
                      ),
                    ),
                  ),
                ),
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
        switch (_to_stream("foo bar") |> Parser.ast) {
        | _ => Assert.fail("expected ParseFailed exception")
        | exception ParseFailed => ()
        }
    ),
  ];
