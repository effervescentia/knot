open Core;
open Knot.Globals;
open Knot.Token;

let print_tkn =
  (
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