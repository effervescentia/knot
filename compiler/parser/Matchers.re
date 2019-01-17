open Knot.Globals;
open Knot.Fiber;
open Knot.Token;
open AST;

let exactly = x => satisfy(((y, _)) => y == x);
let one_of = xs => satisfy(((y, _)) => List.exists(x => x == y, xs));

let space = input => one_of([Space, Tab, Newline], input);
let spaces = input => skip_many(space, input);

let comma_separated = s => sep_by(s, exactly(Comma));

let lexeme = x => spaces >> x;

let token = (x, input) => (exactly(x) |> lexeme)(input);
let kwd = x => token(Keyword(x));

/* keywords */
let import = input => kwd(Import, input)
and from = input => kwd(From, input)
and const = input => kwd(Const, input)
and func = input => kwd(Func, input)
and view = input => kwd(View, input)
and state = input => kwd(State, input)
and style = input => kwd(Style, input)
and get = input => kwd(Get, input)
and mut = input => kwd(Mut, input);

/* characters */
let assign = input => token(Assign, input)
and period = input => token(Period, input)
and comma = input => token(Comma, input)
and colon = input => token(Colon, input)
and semicolon = input => token(Semicolon, input)
and tilde = input => token(Tilde, input)
and dollar_sign = input => token(DollarSign, input)
and number_sign = input => token(NumberSign, input)
and minus = input => token(Minus, input)
and plus = input => token(Plus, input)
and asterisk = input => token(Asterisk, input)
and forward_slash = input => token(ForwardSlash, input)
and vertical_bar = input => token(VerticalBar, input)
and ampersand = input => token(Ampersand, input)
and l_paren = input => token(LeftParenthese, input)
and l_brace = input => token(LeftBrace, input)
and l_brack = input => token(LeftBracket, input)
and r_paren = input => token(RightParenthese, input)
and r_brace = input => token(RightBrace, input)
and r_brack = input => token(RightBracket, input)
and l_chev = input => token(LeftChevron, input)
and r_chev = input => token(RightChevron, input);

/* patterns */
let lambda = input => token(Lambda, input)
and less_than_or_equal = input => token(LessThanOrEqual, input)
and greater_than_or_equal = input => token(GreaterThanOrEqual, input)
and logical_and = input => token(LogicalAnd, input)
and logical_or = input => token(LogicalOr, input)
and jsx_self_close = input => token(JSXSelfClose, input)
and jsx_open_end = input => token(JSXOpenEnd, input)
and jsx_start_frag = input => token(JSXStartFragment, input)
and jsx_end_frag = input => token(JSXEndFragment, input);

let identifier = input =>
  (
    (
      fun
      | LazyStream.Cons((Identifier(s), _), next_in) =>
        Some((s, Lazy.force(next_in)))
      | _ => None
    )
    |> lexeme
  )(
    input,
  );
let exact_identifier = (match, input) =>
  switch (identifier(input)) {
  | Some((s, _)) as res when s == match => res
  | _ => None
  };

let string = input =>
  (
    (
      fun
      | LazyStream.Cons((String(s), _), next_in) =>
        Some((s, Lazy.force(next_in)))
      | _ => None
    )
    |> lexeme
  )(
    input,
  );

let number = input =>
  (
    (
      fun
      | LazyStream.Cons((Number(n), _), next_in) =>
        Some((n, Lazy.force(next_in)))
      | _ => None
    )
    |> lexeme
  )(
    input,
  );

let boolean = input =>
  (
    (
      fun
      | LazyStream.Cons((Boolean(b), _), next_in) =>
        Some((b, Lazy.force(next_in)))
      | _ => None
    )
    |> lexeme
  )(
    input,
  );

let parentheses = input => between(l_paren, r_paren, input);
let braces = input => between(l_brace, r_brace, input);
let brackets = input => between(l_brack, r_brack, input);
let decl = x => x >> identifier;
let terminated = x => x << optional(semicolon);
let closure = x => many(x) |> braces;
let type_def = input =>
  opt(None, colon >> identifier ==> (t => Some(t)), input);
let eof = input => (spaces >> Knot.Fiber.eof())(input);
