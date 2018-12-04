type token =
  | Space
  | Tab
  | Newline
  | Assign
  | Period
  | Comma
  | Colon
  | Semicolon
  | Tilde
  | Plus
  | Minus
  | Asterisk
  | VerticalBar
  | Ampersand
  | ForwardSlash
  | LeftParenthese
  | RightParenthese
  | LeftBrace
  | RightBrace
  | LeftBracket
  | RightBracket
  | LeftChevron
  | RightChevron
  | Lambda
  | Equals
  | GreaterThanOrEqual
  | LessThanOrEqual
  | JSXSelfClose /* /> */
  | JSXOpenEnd /* </ */
  | Number(int)
  | String(string)
  | LineComment(string)
  | BlockComment(string)
  | Unexpected(char);

/* let print_tkn =
   fun
   | Space => " "
   | Tab => "\t"
   | Newline => "\n"
   | Plus => "+"
   | Minus => "-"
   | Asterisk => "*"
   | ForwardSlash => "/"
   | Number(n) => Printf.sprintf("number(%d)", n)
   | String(s) => Printf.sprintf("string(%s)", s)
   | LineComment(s) => Printf.sprintf("line_comment(%s)", s)
   | BlockComment(s) => Printf.sprintf("block_comment(%s)", s)
   | Unexpected(c) => Printf.sprintf("UNEXPECTED(%c)", c); */

exception UnclosedString;
exception UnclosedBlockComment;

let _is_number = ch =>
  int_of_char(ch) >= int_of_char('0')
  && int_of_char(ch) <= int_of_char('9');
let rec _next_non_space = stream =>
  switch (FileStream.peek(stream)) {
  | Some(' ' | '\t' | '\n') =>
    FileStream.junk(stream);
    _next_non_space(stream);
  | Some(tkn) => Some(tkn)
  | None => None
  };
let _peek_next_non_space = (stream, cursor) => {
  let char = _next_non_space(stream);
  FileStream.reposition(stream, cursor);
  char;
};
let _junk_non_space = stream => _next_non_space(stream) |> ignore;
let _lex_pair = (ch, match, no_match, stream, cursor) =>
  switch (_peek_next_non_space(stream, cursor)) {
  | Some(c) when c == ch =>
    _junk_non_space(stream);
    match;
  | _ => no_match
  };

let lex_lambda = _lex_pair('>', Lambda, Minus);
let lex_less_than_or_eql = _lex_pair('=', LessThanOrEqual, LeftChevron);
let lex_greater_than_or_eql =
  _lex_pair('=', GreaterThanOrEqual, RightChevron);
let lex_equals = _lex_pair('=', Equals, Assign);

let rec lex_number = (chs, stream) =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    switch (ch) {
    | _ when _is_number(ch) =>
      FileStream.junk(stream);
      lex_number([ch, ...chs], stream);
    | _ => Util.chs_to_number(chs)
    }
  | None => Util.chs_to_number(chs)
  };

let rec lex_string = (chs, stream) =>
  switch (FileStream.next(stream)) {
  | Some((ch, _)) =>
    switch (ch) {
    | '"' =>
      List.rev(chs)
      |> List.fold_left((acc, c) => acc ++ String.make(1, c), "")
    | _ => lex_string([ch, ...chs], stream)
    }
  | None => raise(UnclosedString)
  };

let rec lex_line_comment = stream =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    switch (ch) {
    | '/' =>
      FileStream.junk(stream);
      lex_block_comment(stream);
    | _ => ForwardSlash
    }
  | None => ForwardSlash
  }
and lex_block_comment = stream =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    switch (ch) {
    | '/' =>
      FileStream.junk(stream);
      BlockComment(_read_until_end_block([], stream));
    | _ => LineComment(_read_until_newline([], stream))
    }
  | None => LineComment(_read_until_newline([], stream))
  }
and _read_until_newline = (chs, stream) =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    switch (ch) {
    | '\n' => Util.chs_to_string(chs)
    | _ =>
      FileStream.junk(stream);
      _read_until_newline([ch, ...chs], stream);
    }
  | None => Util.chs_to_string(chs)
  }
and _read_until_end_block = (chs, stream) =>
  switch (FileStream.npeek(3, stream)) {
  | Some(s) =>
    switch (s) {
    | "///" =>
      FileStream.njunk(3, stream);
      Util.chs_to_string(chs);
    | _ =>
      switch (FileStream.next(stream)) {
      | Some((ch, _)) => _read_until_end_block([ch, ...chs], stream)
      | None => raise(UnclosedBlockComment)
      }
    }
  | None => raise(UnclosedBlockComment)
  };

let lex = ((ch, cursor), stream) =>
  switch (ch) {
  | ' ' => Space
  | '\t' => Tab
  | '\n' => Newline
  | '.' => Period
  | ',' => Comma
  | ':' => Colon
  | ';' => Semicolon
  | '~' => Tilde
  | '+' => Plus
  | '*' => Asterisk
  | '(' => LeftParenthese
  | ')' => RightParenthese
  | '{' => LeftBrace
  | '}' => RightBrace
  | '[' => LeftBracket
  | ']' => RightBracket
  | '|' => VerticalBar
  | '&' => Ampersand
  | '-' => lex_lambda(stream, cursor)
  | '<' => lex_less_than_or_eql(stream, cursor)
  | '>' => lex_greater_than_or_eql(stream, cursor)
  | '=' => lex_equals(stream, cursor)
  | '/' => lex_line_comment(stream)
  | '"' => String(lex_string([], stream))
  | _ when _is_number(ch) => Number(lex_number([ch], stream))
  | _ => Unexpected(ch)
  };