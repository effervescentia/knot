open OUnit2;
open Knot.Token;

module LexTable = KnotLex.LexTable;

let __all_tokens = [
  Plus,
  ForwardSlash,
  Plus,
  Minus,
  Asterisk,
  Space,
  Ampersand,
  ForwardSlash,
  LeftBrace,
  Space,
  Number(9),
  LeftParenthese,
  Number(3),
  LeftChevron,
  Number(2),
  Equals,
  Space,
  Space,
  Number(90),
  Lambda,
  Newline,
  String("BC"),
  Space,
  LeftBracket,
  Minus,
  Plus,
  Asterisk,
  RightChevron,
  ForwardSlash,
  RightBracket,
  GreaterThanOrEqual,
  Space,
  Number(300),
  Assign,
  DollarSign,
  Newline,
  Number(5),
  Space,
  Number(412),
  RightParenthese,
  VerticalBar,
  RightParenthese,
  Space,
  RightBrace,
  LineComment(" dth asd elkjqw"),
  Newline,
  JSXOpenEnd,
  String("123123"),
  Space,
  Equals,
  BlockComment("\n.a41#@\n"),
  Tilde,
  LogicalOr,
  LogicalAnd,
  LessThanOrEqual,
  Assign,
  JSXSelfClose,
  Newline,
  Newline,
  JSXSelfClose,
  LogicalOr,
  JSXOpenEnd,
  Space,
  LogicalAnd,
  Newline,
  Keyword(Main),
  Space,
  Keyword(Import),
  Space,
  Keyword(Const),
  Space,
  Keyword(Let),
  Space,
  Keyword(State),
  Space,
  Keyword(View),
  Space,
  Keyword(Func),
  Space,
  Keyword(If),
  Space,
  Keyword(Else),
  Space,
  Keyword(Get),
  Space,
  Keyword(Mut),
  Newline,
  Identifier("mainer"),
  Space,
  Identifier("importest"),
  Space,
  Identifier("constant"),
  Space,
  Identifier("letter"),
  Space,
  Identifier("stated"),
  Space,
  Identifier("viewing"),
  Space,
  Identifier("functor"),
  Space,
  Identifier("iffer"),
  Space,
  Identifier("elsern"),
  Space,
  Identifier("getting"),
  Space,
  Identifier("mutter"),
  Space,
  Identifier("igloo"),
  Space,
  Identifier("moron"),
];

let test_lex_tokens = (file, _) => {
  let channel = Util.load_resource(file);
  let input = Knot.FileStream.of_channel(channel);

  let rec next = (tkns, stream) =>
    switch (LexTable.next_token(stream), tkns) {
    | (Some((x, next_stream)), [t, ...ts]) =>
      Printf.sprintf("'%s'", KnotLex.Debug.print_tkn(x)) |> print_endline;
      assert_bool("should match expected token", x == t);
      next(ts, next_stream);
    | (None, [t, ...ts]) =>
      assert_failure("lexer did not detect all tokens")
    | (Some(_), []) =>
      assert_failure("lexer detected more tokens than expected")
    | (None, []) => ()
    };

  next(__all_tokens, input);
};

let () =
  run_test_tt_main(
    "LexTable"
    >::: ["lex unix file" >:: test_lex_tokens(Config.unix_tokens_file)],
  );