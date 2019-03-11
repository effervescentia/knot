open Knot.Core;
open Knot.Fiber;

let space = one_of([Space, Tab, Newline]);
let spaces = skip_many(space);

let comma_separated = s => sep_by(s, exactly(Comma));

let lexeme = x => spaces >> x;

let token = (x, input) => (exactly(x) |> lexeme)(input);
let kwd = x => token(Keyword(x));

/* keywords */
let import = kwd(Import)
and from = kwd(From)
and const = kwd(Const)
and func = kwd(Func)
and view = kwd(View)
and state = kwd(State)
and style = kwd(Style)
and as_ = kwd(As)
and get = kwd(Get)
and mut = kwd(Mut)
and main = kwd(Main);

/* characters */
let assign = token(Assign)
and period = token(Period)
and comma = token(Comma)
and colon = token(Colon)
and semicolon = token(Semicolon)
and tilde = token(Tilde)
and dollar_sign = token(DollarSign)
and number_sign = token(NumberSign)
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
and r_brack = token(RightBracket)
and l_chev = token(LeftChevron)
and r_chev = token(RightChevron);

/* patterns */
let lambda = token(Lambda)
and less_than_or_equal = token(LessThanOrEqual)
and greater_than_or_equal = token(GreaterThanOrEqual)
and logical_and = token(LogicalAnd)
and logical_or = token(LogicalOr)
and jsx_self_close = token(JSXSelfClose)
and jsx_open_end = token(JSXOpenEnd)
and jsx_start_frag = token(JSXStartFragment)
and jsx_end_frag = token(JSXEndFragment);

let identifier =
  (
    fun
    | LazyStream.Cons(Identifier(s), next_in) =>
      Some((s, Lazy.force(next_in)))
    | _ => None
  )
  |> lexeme;
let exact_identifier = (match, input) =>
  switch (identifier(input)) {
  | Some((s, _)) as res when s == match => res
  | _ => None
  };

let string =
  (
    fun
    | LazyStream.Cons(String(s), next_in) => Some((s, Lazy.force(next_in)))
    | _ => None
  )
  |> lexeme;

let number =
  (
    fun
    | LazyStream.Cons(Number(n), next_in) => Some((n, Lazy.force(next_in)))
    | _ => None
  )
  |> lexeme;

let number =
  (
    fun
    | LazyStream.Cons(Number(n), next_in) => Some((n, Lazy.force(next_in)))
    | _ => None
  )
  |> lexeme;

let boolean =
  (
    fun
    | LazyStream.Cons(Boolean(b), next_in) => Some((b, Lazy.force(next_in)))
    | _ => None
  )
  |> lexeme;

let parentheses = input => between(l_paren, r_paren, input);
let braces = input => between(l_brace, r_brace, input);
let brackets = input => between(l_brack, r_brack, input);
let decl = x => x >> identifier;
let terminated = x => x << optional(semicolon);
let closure = x => many(x) |> braces;
let type_def = input =>
  (opt(None, colon >> identifier ==> no_ctx % (t => Some(t))))(input);
let eof = spaces >> Knot.Fiber.eof();
