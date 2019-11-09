open Core;

let to_file_stream = s => {
  let decoder = Uutf.decoder(`String(s));
  let next = () =>
    switch (Uutf.decode(decoder)) {
    | `End => None
    | `Uchar(ch) =>
      Some((ch, (Uutf.decoder_line(decoder), Uutf.decoder_col(decoder))))
    | `Malformed(_) => None
    | `Await => assert(false)
    };

  LazyStream.of_function(next);
};

let test_lex_token = ((s, tkn)) => {
  switch (Lexer.next_token(to_file_stream(s))) {
  | Some((t, _)) => assert_tkn_eql(t, tkn)
  | None => assert_failure("no token found")
  };
};
let test_lex_tokens = (xs, _) => List.iter(test_lex_token, xs);

let tests =
  "KnotLex.Lexer"
  >::: [
    "lex invalid character"
    >:: (
      _ =>
        switch (Lexer.next_token(to_file_stream("∑"))) {
        | _ => assert_failure("accepted invalid character")
        | exception (
                      CompilationError(
                        SyntaxError(InvalidCharacter(ch, cursor)),
                      )
                    ) =>
          assert_single_char_cursor_eql(
            (ch, cursor),
            (Uchar.of_int(8721), (1, 1)),
          )
        }
    ),
    "lex unclosed string"
    >:: (
      _ =>
        switch (
          Lexer.next_token(
            to_file_stream(
              "\" this is an unclosed string \nwith new \nlines in it",
            ),
          )
        ) {
        | _ => assert_failure("accepted unclosed string")
        | exception (CompilationError(SyntaxError(UnclosedString(cursor)))) =>
          assert_cursor_eql(cursor, (1, 1))
        }
    ),
    "lex unclosed comment"
    >:: (
      _ =>
        switch (
          Lexer.next_token(
            to_file_stream(
              "/* this is an unclosed comment block \nwith new \nlines in it",
            ),
          )
        ) {
        | _ => assert_failure("accepted unclosed comment")
        | exception (
                      CompilationError(
                        SyntaxError(UnclosedCommentBlock(cursor)),
                      )
                    ) =>
          assert_cursor_eql(cursor, (1, 1))
        }
    ),
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
    "lex boolean"
    >:: test_lex_tokens([
          ("true", Boolean(true)),
          ("false", Boolean(false)),
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
          ("// comment with eof", LineComment(" comment with eof")),
          ("//\n", LineComment("")),
          ("//", LineComment("")),
          (
            "/*931lkj das\n e1;lk312*/",
            BlockComment("931lkj das\n e1;lk312"),
          ),
          ("/**/", BlockComment("")),
        ]),
  ];
