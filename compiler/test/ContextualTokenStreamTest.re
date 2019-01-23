open Core;

open KnotLex.Core;

let __contextual_string = "const abc = <main jsx={with()} nested=\"expressions\">and some{\"raw strings\"}</main>;";

let to_contextual_stream = file =>
  Util.load_resource(file)
  |> FileStream.of_channel
  |> ContextualTokenStream.of_file_stream;

let test_read_fully = (file, expected_tkns, _) => {
  let contextual_stream = to_contextual_stream(file);

  let rec loop = (stream, tkns) =>
    switch (stream, tkns) {
    | (LazyStream.Cons((tkn, _), next_stream), [x, ...xs]) =>
      assert_tkn_eql(tkn, x);
      loop(Lazy.force(next_stream), xs);
    | (LazyStream.Cons(_, _), _) =>
      assert_failure("lexer detected more tokens than expected")
    | (LazyStream.Nil, [x, ...xs]) =>
      assert_failure("lexer did not detect all tokens")
    | (LazyStream.Nil, []) => ()
    };

  loop(contextual_stream, expected_tkns);
};

let test_read_with_context_rules = (str, ctx_rules, expected_tkns, _) => {
  let contextual_stream =
    Util.to_file_stream(str) |> ContextualTokenStream.of_file_stream;

  let rec loop = (stream, tkns) =>
    switch (stream, tkns) {
    | (LazyStream.Cons((tkn, update_ctx), next_stream), [x, ...xs]) =>
      assert_tkn_eql(tkn, x);
      ctx_rules(tkn) |> update_ctx;
      loop(Lazy.force(next_stream), xs);
    | (LazyStream.Cons(_, _), _) =>
      assert_failure("lexer detected more tokens than expected")
    | (LazyStream.Nil, [x, ...xs]) =>
      assert_failure("lexer did not detect all tokens")
    | (LazyStream.Nil, []) => ()
    };

  loop(contextual_stream, expected_tkns);
};

let tests =
  "KnotLex.ContextualTokenStream"
  >::: [
    "read unix file token stream"
    >:: test_read_fully(Config.unix_tokens_file, Fixtures.all_tokens),
    "read windows file token stream"
    >:: test_read_fully(Config.windows_tokens_file, Fixtures.all_tokens),
    "read tokens with context mutation"
    >:: test_read_with_context_rules(
          __contextual_string,
          {
            let start_tag_finished = ref(false);

            fun
            | LeftChevron => PushContext(JSXStartTag)
            | RightChevron =>
              if (start_tag_finished^) {
                PopContext;
              } else {
                start_tag_finished := true;
                SwitchContext(JSXContent);
              }
            | LeftBrace => PushContext(Normal)
            | RightBrace => PopContext
            | JSXOpenEnd => SwitchContext(JSXEndTag)
            | _ => NoOp;
          },
          [
            Keyword(Const),
            Space,
            Identifier("abc"),
            Space,
            Assign,
            Space,
            LeftChevron,
            Identifier("main"),
            Space,
            Identifier("jsx"),
            Assign,
            LeftBrace,
            Identifier("with"),
            LeftParenthese,
            RightParenthese,
            RightBrace,
            Space,
            Identifier("nested"),
            Assign,
            String("expressions"),
            RightChevron,
            JSXTextNode("and some"),
            LeftBrace,
            String("raw strings"),
            RightBrace,
            JSXOpenEnd,
            Identifier("main"),
            RightChevron,
            Semicolon,
          ],
        ),
  ];
