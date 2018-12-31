open OUnit2;
open Knot.Token;
open KnotParse.AST;


let to_token_stream = tkns => {
  let remaining = ref(tkns);
  let next = ts =>
    if (List.length(ts) == 0) {
      None;
    } else {
      let t = List.nth(ts, 0);
      remaining := List.tl(ts);
      Some(t);
    };

  Knot.LazyStream.of_function(() => next(remaining^));
};

let test_parse_ast = ((tkns, ast)) =>
  switch (KnotParse.Parser.parse(to_token_stream(tkns))) {
  | Some(res) => Assert.assert_ast_eql(ast, res)
  | None => assert_failure("no AST found")
  };

let test_parse_asts = xs => List.iter(test_parse_ast, xs);

let tests =
  "KnotParse.Parser"
  >::: [
    "parse main import statement"
    >:: (
      _ => {
        let stmt = [
          Keyword(Import),
          Space,
          Identifier("Table"),
          Space,
          Keyword(From),
          Space,
          String("table"),
        ];
        let expected =
          Statements([Import("table", [MainExport("Table")])]);

        test_parse_asts([
          (stmt, expected),
          (stmt @ [Space, Semicolon], expected),
        ]);
      }
    ),
  ];