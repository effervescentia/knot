open Knot.Token;

let print_tkn =
  fun
  | Space => " "
  | Tab => "tab(\\t)"
  | Newline => "newline(\\n)"
  | Assign => "="
  | Period => "."
  | Comma => ","
  | Colon => ":"
  | Semicolon => ";"
  | Tilde => "~"
  | DollarSign => "$"
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
  | LogicalOr => "||"
  | LogicalAnd => "&&"
  | Keyword(kwd) =>
    switch (kwd) {
    | Main => "keyword(main)"
    | Import => "keyword(import)"
    | Const => "keyword(const)"
    | Let => "keyword(let)"
    | State => "keyword(state)"
    | View => "keyword(view)"
    | Func => "keyword(func)"
    | If => "keyword(if)"
    | Else => "keyword(else)"
    | Get => "keyword(get)"
    | Mut => "keyword(mut)"
    }
  | Identifier(s) => Printf.sprintf("identifier(%s)", s)
  | Number(n) => Printf.sprintf("number(%d)", n)
  | JSXTextNode(s) => Printf.sprintf("jsx_text_node(%s)", s)
  | String(s) => Printf.sprintf("string(%s)", s)
  | LineComment(s) => Printf.sprintf("line_comment(%s)", s)
  | BlockComment(s) => Printf.sprintf("block_comment(%s)", s)
  | Unexpected(c) => Printf.sprintf("UNEXPECTED(%c)", c);