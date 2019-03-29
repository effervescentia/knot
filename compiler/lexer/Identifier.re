open Core;

let underscore = Char('_');
let identifier_matchers = [underscore, AlphaNumeric];

let rec lex_subsequent_chars = (f, reserved) =>
  Lexers([
    Lexer(
      Either(identifier_matchers),
      Either(identifier_matchers),
      _ => lex_subsequent_chars(f, reserved),
    ),
    Lexer(
      Either(identifier_matchers),
      Except(identifier_matchers),
      /* only needed here as there are no 1-character reserved tokens */
      s => List.mem(s, reserved) ? Lexers([]) : f(s),
    ),
  ]);

let identifier_lexer = (~reserved=Core.reserved, ()) =>
  Lexers([
    Lexer(
      Either([underscore, Alpha]),
      Except(identifier_matchers),
      s => Result(Identifier(s)),
    ),
    Lexer(
      Either([underscore, Alpha]),
      Either(identifier_matchers),
      _ => lex_subsequent_chars(x => Result(Identifier(x)), reserved),
    ),
  ]);

let sidecar_lexer =
  Lexer(
    Char('$'),
    Either(identifier_matchers),
    _ =>
      lex_subsequent_chars(
        x =>
          Result(
            SidecarIdentifier(String.sub(x, 1, String.length(x) - 1)),
          ),
        [],
      ),
  );

let lexer = Lexers([sidecar_lexer, identifier_lexer()]);
