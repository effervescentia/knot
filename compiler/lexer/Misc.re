open Knot.Token;

module FileStream = Knot.FileStream;

exception UnclosedBlockComment;

let _lex_pair = (ch, match, no_match, stream, cursor) =>
  switch (Util.peek_next_non_space(stream, cursor)) {
  | Some(c) when c == ch =>
    Util.junk_non_space(stream);
    match;
  | _ => no_match
  };

let lex_minus = _lex_pair('>', Lambda, Minus);
let lex_right_chevron = _lex_pair('=', GreaterThanOrEqual, RightChevron);
let lex_assign = _lex_pair('=', Equals, Assign);
let lex_ampersand = _lex_pair('&', LogicalAnd, Ampersand);
let lex_vertical_bar = _lex_pair('|', LogicalOr, VerticalBar);
let lex_jsx_self_close = _lex_pair('>', JSXSelfClose, Unexpected('/'));

let rec lex_jsx_text_node = (chs, stream) =>
  switch (FileStream.peek(stream)) {
  | None
  | Some('<' | '{') => Util.chs_to_string(chs)
  | Some(ch) =>
    FileStream.junk(stream);
    lex_jsx_text_node([ch, ...chs], stream);
  };

let rec lex_forward_slash = (stream, cursor) =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    switch (ch) {
    | '/' =>
      FileStream.junk(stream);
      lex_block_comment(stream);
    | _ => _lex_pair('>', JSXSelfClose, ForwardSlash, stream, cursor)
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
    switch (s |> Util.chs_to_string) {
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