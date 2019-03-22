open Globals;
open Token;

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
    | QuestionMark => "?"
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
        | Module => "module"
        | From => "from"
        | Import => "import"
        | Const => "const"
        | Let => "let"
        | Func => "func"
        | View => "view"
        | State => "state"
        | Style => "style"
        | If => "if"
        | As => "as"
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
