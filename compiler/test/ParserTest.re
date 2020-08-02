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
          |> TokenStream.of_file_stream(Lexer.next_token);

        switch (Parser.parse(Parser.prog, token_stream)) {
        | Some(res) => Assert.assert_ast_eql(Fixtures.full_ast, res)
        | None => assert_failure("no AST found")
        };
      }
    ),
    "parse empty module"
    >:: (_ => Util.test_parse_ast(Parser.prog, ([], Module([], [])))),
    "parse main import statement"
    >:: (
      _ => {
        let name = "MyImport";
        let module_name = "table";
        let stmt =
          [
            Keyword(Import),
            Identifier(name),
            Keyword(From),
            String(module_name),
          ]
          |> Util.drift;
        let expected =
          Module([Import(module_name, [no_ctx(MainExport(name))])], []);
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
          Module(
            [],
            [
              Declaration(name, no_ctx(ConstDecl(no_ctx(NumericLit(5))))),
            ],
          );
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
          Module([], [Declaration(name, no_ctx(FunctionDecl([], [])))]);
        Util.test_parse_ast(Parser.prog, (stmt, expected));
      }
    ),
    "parse view declaration"
    >:: (
      _ => {
        let name = "MyView";
        let stmt = ViewParserTest.empty_view_decl(name);
        let expected =
          Module(
            [],
            [Declaration(name, no_ctx(ViewDecl(None, [], [], [])))],
          );
        Util.test_parse_ast(Parser.prog, (stmt, expected));
      }
    ),
    "parse state declaration"
    >:: (
      _ => {
        let name = "MyState";
        let stmt = StateParserTest.empty_state_decl(name);
        let expected =
          Module([], [Declaration(name, no_ctx(StateDecl([], [])))]);
        Util.test_parse_ast(Parser.prog, (stmt, expected));
      }
    ),
    ImportParserTest.tests,
    ConstParserTest.tests,
    FunctionParserTest.tests,
    ViewParserTest.tests,
    StateParserTest.tests,
  ];
