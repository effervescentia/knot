open Core;
open Knot.Token;

let rec lex_comment_block = () =>
  Lexers([
    Lexer(Except([Token("///")]), Any, lazy (lex_comment_block())),
    Lexer(
      Token("///"),
      Any,
      lazy (
        Result(s => BlockComment(String.sub(s, 3, String.length(s) - 4)))
      ),
    ),
  ]);

let rec lex_comment = () =>
  Lexers([
    Lexer(Except([Char('\n')]), Any, lazy (lex_comment())),
    Lexer(
      Char('\n'),
      Any,
      lazy (
        Result(s => LineComment(String.sub(s, 2, String.length(s) - 1)))
      ),
    ),
  ]);

let lexer =
  Lexers([
    Lexer(Token("///"), Any, lazy (lex_comment_block())),
    Lexer(Token("//"), Any, lazy (lex_comment())),
  ]);