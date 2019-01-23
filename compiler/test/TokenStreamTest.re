open Core;

let test_read_fully = (file, expected_tkns, _) => {
  let token_stream =
    Util.load_resource(file)
    |> FileStream.of_channel
    |> TokenStream.of_file_stream("", _ => true);

  let rec loop = (stream, tkns) =>
    switch (stream, tkns) {
    | (ContextualStream.Cons(tkn, next_stream, _, _), [x, ...xs]) =>
      assert_tkn_eql(tkn, x);
      loop(Lazy.force(next_stream), xs);
    | (ContextualStream.Cons(_), _) =>
      assert_failure("lexer detected more tokens than expected")
    | (ContextualStream.Nil, [x, ...xs]) =>
      assert_failure("lexer did not detect all tokens")
    | (ContextualStream.Nil, []) => ()
    };

  loop(token_stream, expected_tkns);
};

let tests =
  "KnotLex.TokenStream"
  >::: [
    "read unix file token stream"
    >:: test_read_fully(Config.unix_tokens_file, Fixtures.all_tokens),
    "read windows file token stream"
    >:: test_read_fully(Config.windows_tokens_file, Fixtures.all_tokens),
  ];
