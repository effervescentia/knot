open OUnit2;
open Knot.Token;

module Lexer = KnotLex.Lexer;

let to_file_stream = s => {
  let remaining = ref(s);
  let next = s =>
    if (String.length(s) == 0) {
      None;
    } else {
      let ch = s.[0];
      remaining := String.sub(s, 1, String.length(s) - 1);
      Some((ch, (0, 0)));
    };

  Knot.LazyStream.of_function(() => next(remaining^));
};

let test_lex_tokens = (xs, _) =>
  List.iter(
    ((s, tkn)) =>
      switch (Lexer.next_token(to_file_stream(s))) {
      | Some((t, _)) => Assert.assert_tkn_eql(t, tkn)
      | None => assert_failure("no token found")
      },
    xs,
  );

let () =
  run_test_tt_main(
    "Lexer"
    >::: [
      "lex characters"
      >:: test_lex_tokens([
            (" ", Space),
            ("\t", Tab),
            ("\n", Newline),
            ("=", Assign),
            (".", Period),
            (",", Comma),
            (":", Colon),
            (";", Semicolon),
            ("~", Tilde),
            ("$", DollarSign),
            ("-", Minus),
            ("+", Plus),
            ("*", Asterisk),
            ("/", ForwardSlash),
            ("|", VerticalBar),
            ("&", Ampersand),
            ("(", LeftParenthese),
            (")", RightParenthese),
            ("[", LeftBracket),
            ("]", RightBracket),
            ("{", LeftBrace),
            ("}", RightBrace),
            ("<", LeftChevron),
            (">", RightChevron),
          ]),
      "lex keywords"
      >:: test_lex_tokens([
            ("import", Keyword(Import)),
            ("const", Keyword(Const)),
            ("let", Keyword(Let)),
            ("state", Keyword(State)),
            ("view", Keyword(View)),
            ("func", Keyword(Func)),
            ("else", Keyword(Else)),
            ("if", Keyword(If)),
            ("mut", Keyword(Mut)),
            ("get", Keyword(Get)),
            ("main", Keyword(Main)),
          ]),
      "lex numbers"
      >:: test_lex_tokens([
            ("0", Number(0)),
            ("2", Number(2)),
            ("0412", Number(412)),
          ]),
      "lex strings"
      >:: test_lex_tokens([
            ("\"\"", String("")),
            ("\"abc\"", String("abc")),
            ("\"A 13 $3@0..d/ \t\"", String("A 13 $3@0..d/ \t")),
          ]),
      "lex patterns"
      >:: test_lex_tokens([
            ("& \t&", LogicalAnd),
            ("|\n |", LogicalOr),
            ("-\n>", Lambda),
            ("= \n =", Equals),
            ("<\t=", LessThanOrEqual),
            (">  =", GreaterThanOrEqual),
            ("/   >", JSXSelfClose),
            ("<\t/", JSXOpenEnd),
          ]),
      "lex identifiers"
      >:: test_lex_tokens([
            ("bcalkd", Identifier("bcalkd")),
            ("_13ndas", Identifier("_13ndas")),
            ("impor", Identifier("impor")),
            ("mainline", Identifier("mainline")),
          ]),
      "lex comments"
      >:: test_lex_tokens([
            (
              "// 0dl123jfl dqlkqwe[ e1kme\n",
              LineComment(" 0dl123jfl dqlkqwe[ e1kme"),
            ),
            (
              "///931lkj das\n e1;lk312///",
              BlockComment("931lkj das\n e1;lk312"),
            ),
          ]),
    ],
  );