open OUnit2;
open Knot.Token;
open KnotParse.AST;

module Parser = KnotParse.Parser;

let tests =
  "KnotParse.Parser"
  >::: [
    "parse main import statement"
    >:: (
      _ => {
        let name = "MyImport";
        let module_name = "table";
        let stmt = ImportParserTest.main_import_stmt(name, module_name);
        let expected =
          Statements([Import(module_name, [MainExport(name)])]);

        Util.test_many(
          Util.test_parse_ast(Parser.prog),
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
      }
    ),
    "parse const declaration"
    >:: (
      _ => {
        let name = "myConst";
        let stmt = ConstParserTest.const_decl(name, Number(5));
        let expected =
          Statements([Declaration(ConstDecl(name, NumericLit(5)))]);

        Util.test_many(
          Util.test_parse_ast(Parser.prog),
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
          Util.test_parse_ast(Parser.prog),
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
      }
    ),
    "parse function declaration"
    >:: (
      _ => {
        let name = "abc";
        let stmt = FunctionParserTest.simple_func_decl(name);
        let expected =
          Statements([Declaration(FunctionDecl("abc", [], []))]);

        Util.test_many(
          Util.test_parse_ast(Parser.prog),
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
      }
    ),
    ImportParserTest.tests,
    ConstParserTest.tests,
    FunctionParserTest.tests,
    ViewParserTest.tests,
  ];