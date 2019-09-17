open Core;

let __raw_text = "abcde fgh ijklm\n123\n456 7890-_\n\n135\n3ADLKFnn\ncnm, dlkqwe=31 4/123.,e \n\n  f l;k\n  ";

let _next_or_error = input =>
  switch (input) {
  | LazyStream.Cons(x, lz) => (x, Lazy.force(lz))
  | LazyStream.Nil => assert_failure("stream empty")
  };
let rec _cursor_from = (input, count) =>
  switch (input) {
  | LazyStream.Cons(res, lz) =>
    if (count > 1) {
      _cursor_from(Lazy.force(lz), count - 1);
    } else {
      (res, Lazy.force(lz));
    }
  | LazyStream.Nil => ((Uchar.of_int(0), ((-1), (-1))), LazyStream.Nil)
  };
let _assert_cursor =
    ((actual, _), expected_ch, expected_row, expected_column) =>
  assert_cursor_eql(actual, (expected_ch, (expected_row, expected_column)));

let test_read_fully = (file, ctx) => {
  let channel = Util.load_resource(file);
  let input = UnicodeFileStream.of_channel(channel);

  let rec next = (buf, x) =>
    switch (x) {
    | LazyStream.Cons((ch, _), lz) =>
      Buffer.add_utf_8_uchar(buf, ch);
      next(buf, Lazy.force(lz));
    | LazyStream.Nil => Buffer.contents(buf)
    };
  let full_text = next(Buffer.create(512), input);

  close_in(channel);

  assert_string_eql(full_text, __raw_text);
};

let test_cursor_information = (file, char, position, row, column) => {
  let channel = Util.load_resource(file);

  _assert_cursor(
    _cursor_from(UnicodeFileStream.of_channel(channel), position),
    Uchar.of_char(char),
    row,
    column,
  );

  close_in(channel);
};

let test_reposition = (file, position) => {
  let channel = Util.load_resource(file);
  let input = UnicodeFileStream.of_channel(channel);
  let channel_length = in_channel_length(channel);
  let extra_reads = Random.int(channel_length - position - 1);

  let ((_, target_ref), target_input) = _cursor_from(input, position);
  let (expected, input) = _next_or_error(target_input);

  ignore(_cursor_from(input, extra_reads));

  let (actual, _) = _next_or_error(target_input);
  assert_cursor_eql(actual, expected);

  close_in(channel);
};

let tests =
  "Knot.FileStream"
  >::: [
    "read unix file" >:: test_read_fully(Config.unix_file),
    "read windows file" >:: test_read_fully(Config.windows_file),
    "read unix file and check cursor"
    >:: (
      _ => {
        test_cursor_information(Config.unix_file, 'c', 3, 1, 3);
        test_cursor_information(Config.unix_file, '2', 18, 2, 2);
        test_cursor_information(Config.unix_file, '_', 30, 3, 10);
        test_cursor_information(Config.unix_file, '\n', 31, 4, 0);
      }
    ),
    "read windows file and check cursor"
    >:: (
      _ => {
        test_cursor_information(Config.windows_file, 'c', 3, 1, 3);
        test_cursor_information(Config.windows_file, '2', 18, 2, 2);
        test_cursor_information(Config.windows_file, '_', 30, 3, 10);
        test_cursor_information(Config.windows_file, '\n', 31, 4, 0);
      }
    ),
    "read unix file and reposition cursor"
    >:: (
      _ => {
        test_reposition(Config.unix_file, 4);
        test_reposition(Config.unix_file, 19);
        test_reposition(Config.unix_file, 20);
        test_reposition(Config.unix_file, 79);
      }
    ),
    "read windows file and reposition cursor"
    >:: (
      _ => {
        test_reposition(Config.windows_file, 4);
        test_reposition(Config.windows_file, 19);
        test_reposition(Config.windows_file, 20);
        test_reposition(Config.windows_file, 79);
      }
    ),
  ];
