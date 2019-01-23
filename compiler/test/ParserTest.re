open Core;

let tests =
  "KnotParse.Parser"
  >::: [
    "parse full module"
    >:: (
      _ => {
        let token_stream =
          Util.load_resource(Config.unix_module_file)
          |> FileStream.of_channel
          |> TokenStream.of_file_stream("", _ => true);

        switch (Parser.parse(Parser.prog, token_stream)) {
        | Some(res) => Assert.assert_ast_eql(Fixtures.full_ast, res)
        | None => assert_failure("no AST found")
        };
      }
    ),
    "parse empty module"
    >:: (_ => Util.test_parse_ast(Parser.prog, ([], Statements([])))),
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
    "parse function declaration"
    >:: (
      _ => {
        let name = "myFunc";
        let stmt = FunctionParserTest.empty_func_decl(name);
        let expected =
          Statements([Declaration(FunctionDecl(name, [], []))]);

        Util.test_parse_ast(Parser.prog, (stmt, expected));
      }
    ),
    "parse view declaration"
    >:: (
      _ => {
        let name = "MyView";
        let stmt = ViewParserTest.empty_view_decl(name);
        let expected =
          Statements([Declaration(ViewDecl(name, None, [], [], []))]);

        Util.test_parse_ast(Parser.prog, (stmt, expected));
      }
    ),
    "parse state declaration"
    >:: (
      _ => {
        let name = "MyState";
        let stmt = StateParserTest.empty_state_decl(name);
        let expected = Statements([Declaration(StateDecl(name, [], []))]);

        Util.test_parse_ast(Parser.prog, (stmt, expected));
      }
    ),
    ImportParserTest.tests,
    ConstParserTest.tests,
    FunctionParserTest.tests,
    ViewParserTest.tests,
    StateParserTest.tests,
  ];
