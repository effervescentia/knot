open Core;
open Knot.Token;

let token = (x, input) => (exactly(x) |> lexeme)(input);
let kwd = x => token(Keyword(x));

/* keywords */
let import = kwd(Import)
and from = kwd(From)
and const = kwd(Const)
and state = kwd(State)
and func = kwd(Func)
and view = kwd(View);

/* characters */
let assign = token(Assign)
and period = token(Period)
and comma = token(Comma)
and colon = token(Colon)
and semicolon = token(Semicolon)
and tilde = token(Tilde)
and dollar_sign = token(DollarSign)
and minus = token(Minus)
and plus = token(Plus)
and asterisk = token(Asterisk)
and forward_slash = token(ForwardSlash)
and vertical_bar = token(VerticalBar)
and ampersand = token(Ampersand)
and l_paren = token(LeftParenthese)
and l_brace = token(LeftBrace)
and l_brack = token(LeftBracket)
and r_paren = token(RightParenthese)
and r_brace = token(RightBrace)
and r_brack = token(RightBracket);

/* patterns */
let lambda = token(Lambda);

let identifier =
  (
    fun
    | LazyStream.Cons(Identifier(s), next_in) =>
      Some((s, Lazy.force(next_in)))
    | _ => None
  )
  |> lexeme;

let string =
  (
    fun
    | LazyStream.Cons(String(s), next_in) => Some((s, Lazy.force(next_in)))
    | _ => None
  )
  |> lexeme;

let parentheses = input => between(l_paren, r_paren, input);
let braces = input => between(l_brace, r_brace, input);
let brackets = input => between(l_brack, r_brack, input);