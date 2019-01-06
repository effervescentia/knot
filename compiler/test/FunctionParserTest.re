open Core;

let empty_func_decl = name =>
  [
    Keyword(Func),
    Identifier(name),
    LeftParenthese,
    RightParenthese,
    Lambda,
    LeftBrace,
    RightBrace,
  ]
  |> Util.drift;
let compact_func_decl = (name, expr) =>
  [
    Keyword(Func),
    Identifier(name),
    LeftParenthese,
    RightParenthese,
    Lambda,
    expr,
  ]
  |> Util.drift;
let multi_expr_func_decl = name =>
  [
    Keyword(Func),
    Identifier(name),
    LeftParenthese,
    RightParenthese,
    Lambda,
    LeftBrace,
    Number(8),
    Semicolon,
    Number(7),
    Plus,
    Number(2),
    Semicolon,
    String("palatial"),
    Semicolon,
    RightBrace,
  ]
  |> Util.drift;

let test_parse_func = Util.test_parse_decl(KnotParse.Function.decl);

let __name = "myFunction";

let tests =
  "KnotParse.Function"
  >::: [
    "parse empty"
    >:: (
      _ =>
        test_parse_func((
          empty_func_decl(__name),
          FunctionDecl(__name, [], []),
        ))
    ),
    "parse compact"
    >:: (
      _ =>
        test_parse_func((
          compact_func_decl(__name, Number(3)),
          FunctionDecl(__name, [], [NumericLit(3)]),
        ))
    ),
    "parse mult-expression"
    >:: (
      _ =>
        test_parse_func((
          multi_expr_func_decl(__name),
          FunctionDecl(
            __name,
            [],
            [
              NumericLit(8),
              AddExpr(NumericLit(7), NumericLit(2)),
              StringLit("palatial"),
            ],
          ),
        ))
    ),
  ];