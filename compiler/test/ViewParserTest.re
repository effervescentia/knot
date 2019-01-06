open Core;

let empty_view_decl = name =>
  [
    Keyword(View),
    Identifier(name),
    LeftParenthese,
    RightParenthese,
    Lambda,
    LeftBrace,
    RightBrace,
  ]
  |> Util.drift;
let no_params_view_decl = name =>
  [Keyword(View), Identifier(name), Lambda, LeftBrace, RightBrace]
  |> Util.drift;

let test_parse_view = Util.test_parse_decl(KnotParse.View.decl);

let __name = "MyView";

let tests =
  "KnotParse.View"
  >::: [
    "parse empty"
    >:: (
      _ => test_parse_view((empty_view_decl(__name), ViewDecl(__name, [])))
    ),
    "parse no params"
    >:: (
      _ =>
        test_parse_view((no_params_view_decl(__name), ViewDecl(__name, [])))
    ),
  ];