open OUnit2;
open Knot.Token;
open KnotParse.AST;

let to_token_stream = tkns => {
  let remaining = ref(tkns);
  let next = ts =>
    if (List.length(ts) == 0) {
      None;
    } else {
      let t = List.nth(ts, 0);
      remaining := List.tl(ts);
      Some(t);
    };

  Opal.LazyStream.of_function(() => next(remaining^));
};

let test_parse_ast = ((tkns, ast)) =>
  switch (KnotParse.Parser.parse(to_token_stream(tkns))) {
  | Some(res) => Assert.assert_ast_eql(ast, res)
  | None => assert_failure("no AST found")
  };

let test_parse_asts = xs => List.iter(test_parse_ast, xs);

let tests =
  "KnotParse.Parser"
  >::: [
    "parse main import statement"
    >:: (
      _ => {
        let stmt = [
          Keyword(Import),
          Space,
          Identifier("Table"),
          Space,
          Keyword(From),
          Space,
          String("table"),
        ];
        let expected =
          Statements([Import("table", [MainExport("Table")])]);

        test_parse_asts([
          (stmt, expected),
          (stmt @ [Space, Semicolon], expected),
        ]);
      }
    ),
    "parse const declaration"
    >:: (
      _ => {
        let stmt = [
          Keyword(Const),
          Space,
          Identifier("abc"),
          Space,
          Assign,
          Space,
          String("table"),
        ];
        let expected = Statements([Declaration(ConstDecl("abc"))]);

        test_parse_asts([
          (stmt, expected),
          (stmt @ [Space, Semicolon], expected),
        ]);
      }
    ),
    "parse state declaration"
    >:: (
      _ => {
        let stmt = [
          Keyword(State),
          Space,
          Identifier("abc"),
          Space,
          Assign,
          Space,
          String("table"),
        ];
        let expected = Statements([Declaration(StateDecl("abc"))]);

        test_parse_asts([
          (stmt, expected),
          (stmt @ [Space, Semicolon], expected),
        ]);
      }
    ),
    "parse function declaration"
    >:: (
      _ => {
        let stmt = [
          Keyword(Func),
          Space,
          Identifier("abc"),
          Space,
          Assign,
          Space,
          String("table"),
        ];
        let expected = Statements([Declaration(FunctionDecl("abc"))]);

        test_parse_asts([
          (stmt, expected),
          (stmt @ [Space, Semicolon], expected),
        ]);
      }
    ),
    "parse view declaration"
    >:: (
      _ => {
        let stmt = [
          Keyword(View),
          Space,
          Identifier("abc"),
          Space,
          Assign,
          Space,
          String("table"),
        ];
        let expected = Statements([Declaration(ViewDecl("abc"))]);

        test_parse_asts([
          (stmt, expected),
          (stmt @ [Space, Semicolon], expected),
        ]);
      }
    ),
  ];