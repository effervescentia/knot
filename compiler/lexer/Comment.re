open Core;
open Knot.Token;

let rec lex_comment_block = _ =>
  Lexers([
    Lexer(Except([Token("///")]), Any, lex_comment_block),
    Lexer(
      Token("///"),
      Any,
      s => Result(BlockComment(String.sub(s, 3, String.length(s) - 6))),
    ),
  ]);

let rec lex_comment_line = _ =>
  Lexers([
    Lexer(Except([Char('\n')]), Any, lex_comment_line),
    Lexer(
      Except([Char('\n')]),
      Char('\n'),
      s => Result(LineComment(String.sub(s, 2, String.length(s) - 2))),
    ),
  ]);

let lexer =
  Lexers([
    Lexer(Token("///"), Any, lex_comment_block),
    Lexer(Token("//"), Any, lex_comment_line),
  ]);