open OUnit2;
open Knot.Token;
open KnotParse.AST;

module Parser = KnotParse.Parser;

let test_parse_ast = ((tkns, ast)) =>
  switch (Parser.parse(Parser.prog, Util.to_token_stream(tkns))) {
  | Some(res) => Assert.assert_ast_eql(ast, res)
  | None => assert_failure("no AST found")
  };

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

        Util.test_many(
          test_parse_ast,
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
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

        Util.test_many(
          test_parse_ast,
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
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

        Util.test_many(
          test_parse_ast,
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
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

        Util.test_many(
          test_parse_ast,
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
      }
    ),
    ViewParserTest.tests,
  ];