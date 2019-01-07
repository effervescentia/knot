open Core;

let underscore = Char('_');
let identifier_matchers = [underscore, AlphaNumeric];

let rec lex_subsequent_chars = () =>
  Lexers([
    Lexer(
      Either(identifier_matchers),
      Either(identifier_matchers),
      _ => lex_subsequent_chars(),
    ),
    Lexer(
      Either(identifier_matchers),
      Except(identifier_matchers),
      /* only needed here as there are no 1-character reserved tokens */
      s => List.mem(s, Core.reserved) ? Lexers([]) : Result(Identifier(s)),
    ),
  ]);

let lexer =
  Lexers([
    Lexer(
      Either([underscore, Alpha]),
      Except(identifier_matchers),
      s => Result(Identifier(s)),
    ),
    Lexer(
      Either([underscore, Alpha]),
      Either(identifier_matchers),
      _ => lex_subsequent_chars(),
    ),
  ]);