open Core;

let __full_ast =
  Statements([
    Declaration(ConstDecl("numericConst", NumericLit(8))),
    Declaration(
      ConstDecl("additionConst", AddExpr(NumericLit(1), NumericLit(10))),
    ),
    Declaration(
      ConstDecl("subtractionConst", SubExpr(NumericLit(8), NumericLit(2))),
    ),
    Declaration(
      ConstDecl(
        "multiplicationConst",
        MulExpr(NumericLit(2), NumericLit(3)),
      ),
    ),
    Declaration(
      ConstDecl("divisionConst", DivExpr(NumericLit(4), NumericLit(2))),
    ),
    Declaration(ConstDecl("stringConst", StringLit("Hello, World!"))),
    Declaration(ConstDecl("trueConst", BooleanLit(true))),
    Declaration(ConstDecl("falseConst", BooleanLit(false))),
    Declaration(
      ConstDecl("lessThanConst", LTExpr(NumericLit(7), NumericLit(9))),
    ),
    Declaration(
      ConstDecl(
        "lessThanEqualConst",
        LTEExpr(NumericLit(8), NumericLit(2)),
      ),
    ),
    Declaration(
      ConstDecl("greaterThanConst", GTExpr(NumericLit(2), NumericLit(4))),
    ),
    Declaration(
      ConstDecl(
        "greaterThanEqualConst",
        GTEExpr(NumericLit(9), NumericLit(1)),
      ),
    ),
    Declaration(
      ConstDecl(
        "closureConst",
        AddExpr(
          MulExpr(NumericLit(3), NumericLit(2)),
          AddExpr(
            NumericLit(1),
            MulExpr(
              DivExpr(
                NumericLit(6),
                SubExpr(NumericLit(2), NumericLit(5)),
              ),
              NumericLit(3),
            ),
          ),
        ),
      ),
    ),
    Declaration(FunctionDecl("compactFunc", [], [NumericLit(4)])),
    Declaration(
      FunctionDecl(
        "compactExprFunc",
        [],
        [AddExpr(Variable("A"), Variable("B"))],
      ),
    ),
    Declaration(
      FunctionDecl(
        "multiExprFunc",
        [],
        [AddExpr(Variable("e"), Variable("f")), Variable("j")],
      ),
    ),
    Declaration(StateDecl("NoParamsState", [], [])),
    Declaration(StateDecl("EmptyState", [], [])),
    Declaration(
      StateDecl(
        "DefaultParamState",
        [("z", None, Some(NumericLit(30)))],
        [],
      ),
    ),
    Declaration(
      StateDecl(
        "ComplexState",
        [],
        [
          Property("a", Some("b"), None),
          Property("_c", Some("d"), None),
          Getter("e", [], [NumericLit(4)]),
          Getter("f", [], [NumericLit(5)]),
          Getter(
            "g",
            [
              ("h", Some("j"), None),
              ("k", None, Some(NumericLit(2))),
              ("l", Some("m"), Some(NumericLit(20))),
            ],
            [NumericLit(5)],
          ),
          Getter(
            "n",
            [],
            [NumericLit(3), AddExpr(Variable("A"), Variable("m"))],
          ),
        ],
      ),
    ),
    Declaration(ViewDecl("NoParamsView", None, [], [], [])),
    Declaration(ViewDecl("ParamView", None, [], [("m", None, None)], [])),
    Declaration(
      ViewDecl("TypedParamView", None, [], [("a", Some("b"), None)], []),
    ),
    Declaration(
      ViewDecl(
        "DefaultParamView",
        None,
        [],
        [("a", None, Some(NumericLit(4)))],
        [],
      ),
    ),
    Declaration(
      ViewDecl(
        "MultiParamView",
        None,
        [],
        [("m", Some("n"), None), ("a", Some("b"), Some(NumericLit(2)))],
        [],
      ),
    ),
    Declaration(
      ViewDecl(
        "InheritingView",
        Some("SuperView"),
        [],
        [],
        [AddExpr(Variable("a"), Variable("b")), NumericLit(8)],
      ),
    ),
    Declaration(ViewDecl("MixinView", None, ["MyMixin"], [], [])),
    Declaration(
      ViewDecl(
        "InheritingMixinView",
        Some("SuperView"),
        ["MyMixin"],
        [],
        [],
      ),
    ),
    Declaration(
      ViewDecl(
        "ComplexView",
        Some("SuperView"),
        ["MyMixin", "OtherMixin"],
        [],
        [AddExpr(Variable("e"), Variable("f"))],
      ),
    ),
    Declaration(
      StyleDecl(
        "ClassStyle",
        [],
        [
          (
            ClassKey("root"),
            [
              ("fontSize", Protocol("px", [NumericLit(20)])),
              ("backgroundColor", Preset("red")),
            ],
          ),
        ],
      ),
    ),
    Declaration(
      StyleDecl(
        "IdStyle",
        [],
        [
          (
            IdKey("login"),
            [
              ("visibility", Preset("hidden")),
              ("display", Preset("flex")),
            ],
          ),
        ],
      ),
    ),
  ]);

let tests =
  "KnotParse.Parser"
  >::: [
    "parse full module"
    >:: (
      _ => {
        let token_stream =
          Util.load_resource(Config.unix_module_file)
          |> FileStream.of_channel
          |> TokenStream.of_file_stream
          |> TokenStream.without_comments;

        switch (Parser.parse(Parser.prog, token_stream)) {
        | Some(res) => Assert.assert_ast_eql(__full_ast, res)
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