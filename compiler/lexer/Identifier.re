open Core;
open Knot.Token;

let underscore = Char('_');
let identifier_matchers = [underscore, AlphaNumeric];

let rec lex_subsequent_chars = () =>
  Lexers([
    Lexer(
      Either(identifier_matchers),
      Either(identifier_matchers),
      lazy (lex_subsequent_chars()),
    ),
    Lexer(
      Either(identifier_matchers),
      Except(identifier_matchers),
      lazy (Result(s => Identifier(s))),
    ),
  ]);

let lexer =
  Lexers([
    Lexer(
      Either([underscore, Alpha]),
      Except(identifier_matchers),
      lazy (Result(s => Identifier(s))),
    ),
    Lexer(
      Either([underscore, Alpha]),
      Either(identifier_matchers),
      lazy (lex_subsequent_chars()),
    ),
  ]);