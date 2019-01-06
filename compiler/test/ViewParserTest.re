open OUnit2;
open Knot.Token;
open KnotParse.AST;

module Parser = KnotParse.Parser;

let simple_view_decl = name => [
  Keyword(View),
  Space,
  Identifier(name),
  LeftParenthese,
  RightParenthese,
  Space,
  Lambda,
  Space,
  LeftBrace,
  RightBrace,
];

let tests =
  "KnotParse.View"
  >::: [
    "parse declaration"
    >:: (
      _ => {
        let name = "myView";
        let expected = ViewDecl(name);

        Util.test_parse_decl(
          KnotParse.View.decl,
          (simple_view_decl(name), expected),
        );
      }
    ),
  ];