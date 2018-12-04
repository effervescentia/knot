open Tigris;
open OUnit2;
open Assert;

let __unix_file = "unix_file.txt";
let __windows_file = "windows_file.txt";
let __raw_text = "abcde fgh ijklm\n123\n456 7890-_\n\n135\n3ADLKFnn\ncnm, dlkqwe=31 4/123.,e \n\n  f l;k\n  ";

let _load = file => FileStream.load(Config.resource_dir ++ "/" ++ file);
let _next_or_error = stream =>
  switch (FileStream.next(stream)) {
  | Some(x) => x
  | None => assert_failure("stream empty")
  };
let rec _cursor_from = (stream, count) =>
  if (count > 1) {
    FileStream.junk(stream);
    _cursor_from(stream, count - 1);
  } else {
    _next_or_error(stream);
  };
let _assert_cursor =
    (
      (ch, {index, row, column}): (char, FileStream.file_cursor),
      expected_char,
      expected_index,
      expected_row,
      expected_column,
    ) => {
  assert_char_eql(ch, expected_char);
  assert_int_eql(index, expected_index);
  assert_int_eql(row, expected_row);
  assert_int_eql(column, expected_column);
};

let test_read_fully = (file, _) => {
  let stream = _load(file);

  let rec next = s =>
    switch (FileStream.next(stream)) {
    | Some((ch, _)) => next(s ++ String.make(1, ch))
    | None => s
    };
  let full_text = next("");

  assert_string_eql(full_text, __raw_text);

  FileStream.close(stream);
};

let test_cursor_information = (file, char, position, index, row, column) => {
  let stream = _load(file);

  _assert_cursor(_cursor_from(stream, position), char, index, row, column);

  FileStream.close(stream);
};

let test_reposition = (file, position) => {
  Random.self_init();
  let stream = _load(file);
  let channel_length = in_channel_length(stream.channel);
  let extra_reads = Random.int(channel_length - position - 1);

  let rec next = count =>
    if (count > 1) {
      FileStream.junk(stream);
      next(count - 1);
    } else {
      _next_or_error(stream);
    };

  let (_, target_cursor) = next(position);
  let (expected_ch, expected_cursor) = _next_or_error(stream);

  let rec junk = count =>
    if (count > 1) {
      FileStream.junk(stream);
      junk(count - 1);
    } else {
      ();
    };
  junk(extra_reads);

  FileStream.reposition(stream, target_cursor);

  let (actual_ch, actual_cursor) = _next_or_error(stream);
  assert_char_eql(actual_ch, expected_ch);
  assert_int_eql(actual_cursor.index, expected_cursor.index);
  assert_int_eql(actual_cursor.row, expected_cursor.row);
  assert_int_eql(actual_cursor.column, expected_cursor.column);

  FileStream.close(stream);
};

let () =
  run_test_tt_main(
    "FileStream"
    >::: [
      "read file with unix endlines" >:: test_read_fully(__unix_file),
      "read file with windows endlines" >:: test_read_fully(__windows_file),
      "read file and check cursor with unix endlines"
      >:: (
        _ => {
          test_cursor_information(__unix_file, 'c', 3, 3, 1, 3);
          test_cursor_information(__unix_file, '2', 18, 18, 2, 2);
          test_cursor_information(__unix_file, '_', 30, 30, 3, 10);
          test_cursor_information(__unix_file, '\n', 31, 31, 3, 11);
        }
      ),
      "read file and check cursor with windows endlines"
      >:: (
        _ => {
          test_cursor_information(__windows_file, 'c', 3, 3, 1, 3);
          test_cursor_information(__windows_file, '2', 18, 19, 2, 2);
          test_cursor_information(__windows_file, '_', 30, 32, 3, 10);
          test_cursor_information(__windows_file, '\n', 31, 34, 3, 12);
        }
      ),
      "read file and reposition cursor with unix endlines"
      >:: (
        _ => {
          test_reposition(__unix_file, 4);
          test_reposition(__unix_file, 19);
          test_reposition(__unix_file, 20);
          test_reposition(__unix_file, 79);
        }
      ),
      "read file and reposition cursor with windows endlines"
      >:: (
        _ => {
          test_reposition(__windows_file, 4);
          test_reposition(__windows_file, 19);
          test_reposition(__windows_file, 20);
          test_reposition(__windows_file, 80);
        }
      ),
    ],
  );