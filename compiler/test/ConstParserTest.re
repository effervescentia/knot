open Core;

let const_decl = (name, expr) =>
  [Keyword(Const), Identifier(name), Assign, expr] |> Util.drift;

let tests =
  "KnotParse.Const"
  >::: [
    "parse declaration"
    >:: (
      _ => {
        let name = "myConst";
        let value = "table";
        let stmt = const_decl(name, String(value));
        let expected = ConstDecl(name, StringLit(value));

        Util.test_many(
          Util.test_parse_decl(KnotParse.Const.decl),
          [(stmt, expected), (stmt @ [Space, Semicolon], expected)],
        );
      }
    ),
  ];