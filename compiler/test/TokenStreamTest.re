open OUnit2;
open Assert;
open Knot.Lexer;

module TokenStream = Knot.TokenStream;

let __unix_file = "unix_tokens.txt";
let __windows_file = "windows_tokens.txt";
let __all_tokens = [
  Plus,
  ForwardSlash,
  Plus,
  Minus,
  Asterisk,
  Space,
  ForwardSlash,
  Space,
  Number(932),
  Space,
  Space,
  Number(90),
  Newline,
  String("BC"),
  Space,
  Minus,
  Plus,
  Asterisk,
  ForwardSlash,
  Space,
  Number(300),
  Newline,
  Number(5),
  Space,
  Number(412),
  Space,
  LineComment(" dth asd elkjqw"),
  Newline,
  String("123123"),
  Space,
  BlockComment("\n.a41#@\n"),
];

let test_read_fully = (file, expected_tkns, _) => {
  let stream = Util.load_resource(file) |> TokenStream.load;

  let rec loop = tkns =>
    switch (TokenStream.next(stream)) {
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
        "expected tokens to match",
        List.nth(actual_tkns, i - 1) == List.nth(expected_tkns, i - 1),
      );
      assert_loop(i - 1);
    };
  assert_loop(List.length(actual_tkns));
};

let () =
  run_test_tt_main(
    "TokenStream"
    >::: [
      "read unix file token stream"
      >:: test_read_fully(__unix_file, __all_tokens),
      "read windows file token stream"
      >:: test_read_fully(__windows_file, __all_tokens),
    ],
  );