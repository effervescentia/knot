open Core;

let _underscore = Char('_');
let _identifier_matchers = [_underscore, AlphaNumeric];

let rec lex_subsequent_chars = (f, reserved) =>
  Lexers([
    Lexer(
      Either(_identifier_matchers),
      Either(_identifier_matchers),
      _ => lex_subsequent_chars(f, reserved),
    ),
    Lexer(
      Either(_identifier_matchers),
      Except(_identifier_matchers),
      /* only needed here as there are no 1-character reserved tokens */
      s => List.mem(s, reserved) ? Lexers([]) : f(s),
    ),
  ]);

let identifier_lexer = (~reserved=Knot.Constants.reserved_keywords, ()) =>
  Lexers([
    Lexer(
      Either([_underscore, Alpha]),
      Except(_identifier_matchers),
      s => Result(Identifier(s)),
    ),
    Lexer(
      Either([_underscore, Alpha]),
      Either(_identifier_matchers),
      _ => lex_subsequent_chars(x => Result(Identifier(x)), reserved),
    ),
  ]);

let sidecar_lexer =
  Lexer(
    Char('$'),
    Either(_identifier_matchers),
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
