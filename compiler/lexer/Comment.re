open Core;
open Knot.Token;

let triple_slash = Token("///");

let rec lex_comment_block = _ =>
  Lexers([
    Lexer(Except([triple_slash]), Any, lex_comment_block),
    Lexer(
      triple_slash,
      Any,
      s => Result(BlockComment(String.sub(s, 3, String.length(s) - 6))),
    ),
  ]);

let rec lex_comment_line = _ =>
  Lexers([
    Lexer(Except([newline]), Any, lex_comment_line),
    Lexer(
      Except([newline]),
      newline,
      s => Result(LineComment(String.sub(s, 2, String.length(s) - 2))),
    ),
  ]);

let lexer =
  Lexers([
    Lexer(triple_slash, Any, lex_comment_block),
    Lexer(Token("//"), Any, lex_comment_line),
  ]);