open OUnit2;
open Assert;
open Knot.Lexer;

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

let test_read_fully = (file, expected_tkns, _) => {
  let stream = Util.load_resource(file) |> Knot.TokenStream.load;

  let rec loop = tkns =>
    switch (Knot.TokenStream.next(stream)) {
    | Some(tkn) => loop([tkn, ...tkns])
    | None => tkns
    };
  let actual_tkns = loop([]) |> List.rev;

  assert_int_eql(List.length(actual_tkns), List.length(expected_tkns));

  let rec assert_loop = i =>
    if (i == 0) {
      ();
    } else {
      assert_bool(
        Printf.sprintf(
          "expected tokens { %s } and { %s } to match",
          print_tkn(List.nth(actual_tkns, i - 1)),
          print_tkn(List.nth(expected_tkns, i - 1)),
        ),
        List.nth(actual_tkns, i - 1) == List.nth(expected_tkns, i - 1),
      );
      assert_loop(i - 1);
    };
  assert_loop(List.length(actual_tkns));
};

let () =
  run_test_tt_main(
    "Knot.TokenStream"
    >::: [
      "read unix file token stream"
      >:: test_read_fully(Config.unix_tokens_file, __all_tokens),
      "read windows file token stream"
      >:: test_read_fully(Config.windows_tokens_file, __all_tokens),
    ],
  );