open Core;

let print_tkn =
  (
    fun
    | Space => "space( )"
    | Tab => "tab(\t)"
    | Newline => "newline(\n)"
    | Assign => "="
    | Period => "."
    | Comma => ","
    | Colon => ":"
    | Semicolon => ";"
    | Tilde => "~"
    | DollarSign => "$"
    | NumberSign => "#"
    | Plus => "+"
    | Minus => "-"
    | Asterisk => "*"
    | VerticalBar => "|"
    | Ampersand => "&"
    | ForwardSlash => "/"
    | LeftParenthese => "("
    | RightParenthese => ")"
    | LeftBrace => "{"
    | RightBrace => "}"
    | LeftBracket => "["
    | RightBracket => "]"
    | LeftChevron => "<"
    | RightChevron => ">"
    | Lambda => "->"
    | Equals => "=="
    | GreaterThanOrEqual => ">="
    | LessThanOrEqual => "<="
    | JSXSelfClose => "/>"
    | JSXOpenEnd => "</"
    | JSXStartFragment => "<>"
    | JSXEndFragment => "</>"
    | LogicalOr => "||"
    | LogicalAnd => "&&"
    | Keyword(kwd) =>
      (
        switch (kwd) {
        | Main => "main"
        | From => "from"
        | Import => "import"
        | Const => "const"
        | Let => "let"
        | Func => "func"
        | View => "view"
        | State => "state"
        | Style => "style"
        | If => "if"
        | Else => "else"
        | Get => "get"
        | Mut => "mut"
        }
      )
      |> Printf.sprintf("keyword(%s)")
    | Identifier(s) => Printf.sprintf("identifier(%s)", s)
    | Number(n) => Printf.sprintf("number(%d)", n)
    | Boolean(b) => Printf.sprintf("boolean(%b)", b)
    | JSXTextNode(s) => Printf.sprintf("jsx_text_node(%s)", s)
    | String(s) => Printf.sprintf("string(%s)", s)
    | LineComment(s) => Printf.sprintf("line_comment(%s)", s)
    | BlockComment(s) => Printf.sprintf("block_comment(%s)", s)
    | Unexpected(c) => Printf.sprintf("UNEXPECTED(%c)", c)
  )
  % String.escaped;

let rec print_matcher = m =>
  switch (m) {
  | Char(c) =>
    Printf.sprintf("CHAR('%s')", String.make(1, c)) |> String.escaped
  | Token(s) => Printf.sprintf("TOKEN(%s)", s) |> String.escaped
  | Alpha => "ALPHA"
  | Numeric => "NUMERIC"
  | AlphaNumeric => "ALPHA_NUMERIC"
  | Any => "ANY"
  | Either(ms) =>
    Printf.sprintf(
      "EITHER(%s)",
      List.fold_left((acc, m) => acc ++ "|" ++ print_matcher(m), "", ms),
    )
  | Except(ms) =>
    Printf.sprintf(
      "EXCEPT(%s)",
      List.fold_left((acc, m) => acc ++ "|" ++ print_matcher(m), "", ms),
    )
  };

let rec print_lex_table = l =>
  switch (l) {
  | Lexers(ls) =>
    List.fold_left((acc, l) => acc ++ print_lex_table(l), "", ls)
  | Lexer(m, nm, _) =>
    Printf.sprintf("%s >> %s\n", print_matcher(m), print_matcher(nm))
  | Result(x) => "RESULT!\n"
  };

let print_token_stream = token_stream => {
  let rec loop = stream =>
    switch (stream) {
    | LazyStream.Cons(tkn, next_stream) =>
      print_tkn(tkn) ++ loop(Lazy.force(next_stream))
    | LazyStream.Nil => ""
    };

  loop(token_stream);
};

let print_context =
  fun
  | Normal => "normal"
  | JSXContent => "JSX content"
  | JSXStartTag => "JSX start tag"
  | JSXEndTag => "JSX end tag";

let print_action =
  fun
  | NoOp => "no-op"
  | PushContext(ctx) => print_context(ctx) |> Printf.sprintf("push(%s)")
  | SwapContext(ctx) => print_context(ctx) |> Printf.sprintf("swap(%s)")
  | PopContext => "pop";
