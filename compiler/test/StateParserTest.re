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
let state_prop_decl = name =>
  [
    Keyword(State),
    Identifier(name),
    LeftParenthese,
    RightParenthese,
    LeftBrace,
    Identifier("initial"),
    Assign,
    Number(4),
    RightBrace,
  ]
  |> Util.drift;
let state_getter_decl = name =>
  [
    Keyword(State),
    Identifier(name),
    LeftParenthese,
    RightParenthese,
    LeftBrace,
    Keyword(Get),
    Identifier("title"),
    Lambda,
    String("Apple"),
    RightBrace,
  ]
  |> Util.drift;

let test_parse_state = Util.test_parse_decl(KnotParse.State.decl);

let __name = "MyState";

let tests =
  "KnotParse.State"
  >::: [
    "parse empty"
    >:: (
      _ =>
        test_parse_state((
          empty_state_decl(__name),
          StateDecl(__name, [], []),
        ))
    ),
    "parse no params"
    >:: (
      _ =>
        test_parse_state((
          no_params_state_decl(__name),
          StateDecl(__name, [], []),
        ))
    ),
    "parse with property"
    >:: (
      _ =>
        test_parse_state((
          state_prop_decl(__name),
          StateDecl(
            __name,
            [],
            [
              no_ctx(
                Property(
                  no_ctx(("initial", None, Some(no_ctx(NumericLit(4))))),
                ),
              ),
            ],
          ),
        ))
    ),
    "parse with getter"
    >:: (
      _ =>
        test_parse_state((
          state_getter_decl(__name),
          StateDecl(
            __name,
            [],
            [no_ctx(Getter("title", [], [no_ctx(StringLit("Apple"))]))],
          ),
        ))
    ),
  ];
