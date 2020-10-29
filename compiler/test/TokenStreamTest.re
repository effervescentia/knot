open Globals;

let test_read_fully = (file, expected_tkns, _) => {
  let token_stream =
    Util.load_resource(file)
    |> FileStream.of_channel
    |> TokenStream.of_file_stream(~filter=None, Lexer.next_token);

  let rec loop = (stream, tkns) =>
    switch (stream, tkns) {
    | (LazyStream.Cons(tkn, next_stream), [x, ...xs]) =>
      assert_tkn_eql(tkn, x);
      loop(Lazy.force(next_stream), xs);
    | (LazyStream.Cons(_, _), _) =>
      assert_failure("lexer detected more tokens than expected")
    | (LazyStream.Nil, [x, ...xs]) =>
      assert_failure("lexer did not detect all tokens")
    | (LazyStream.Nil, []) => ()
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
