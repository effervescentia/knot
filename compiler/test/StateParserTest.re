open Core;

let empty_state_decl = name =>
  [
    Keyword(State),
    Identifier(name),
    LeftParenthese,
    RightParenthese,
    LeftBrace,
    RightBrace,
  ]
  |> Util.drift;
let no_params_state_decl = name =>
  [Keyword(State), Identifier(name), LeftBrace, RightBrace] |> Util.drift;

let test_parse_state = Util.test_parse_decl(KnotParse.State.decl);

let __name = "MyState";

let tests =
  "KnotParse.State"
  >::: [
    "parse empty"
    >:: (
      _ => test_parse_state((empty_state_decl(__name), StateDecl(__name)))
    ),
    "parse no params"
    >:: (
      _ =>
        test_parse_state((no_params_state_decl(__name), StateDecl(__name)))
    ),
  ];