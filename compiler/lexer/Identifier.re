open Core;

let underscore = Char('_');
let identifier_matchers = [underscore, AlphaNumeric];

let rec lex_subsequent_chars = reserved =>
  Lexers([
    Lexer(
      Either(identifier_matchers),
      Either(identifier_matchers),
      _ => lex_subsequent_chars(reserved),
    ),
    Lexer(
      Either(identifier_matchers),
      Except(identifier_matchers),
      /* only needed here as there are no 1-character reserved tokens */
      s => List.mem(s, reserved) ? Lexers([]) : Result(Identifier(s)),
    ),
  ]);

let lexer = (~reserved=Core.reserved, ()) =>
  Lexers([
    Lexer(
      Either([underscore, Alpha]),
      Except(identifier_matchers),
      s => Result(Identifier(s)),
    ),
    Lexer(
      Either([underscore, Alpha]),
      Either(identifier_matchers),
      _ => lex_subsequent_chars(reserved),
    ),
  ]);
