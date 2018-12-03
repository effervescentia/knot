open Tigris;
open OUnit2;

let __unix_file = "unix_file.txt";
let __windows_file = "windows_file.txt";
let __raw_text = "abcde fgh ijklm\n123\n456 7890__\n\n135\n3ADLKFnn\ncnm, dlkqwe=31 4/123.,e \n\n  f l;k\n  ";

let _load = file => FileStream.load(Config.resource_dir ++ "/" ++ file);
let _assert_cursor =
    (
      {index, row, column}: FileStream.file_cursor,
      expected_index,
      expected_row,
      expected_column,
    ) => {
  assert_bool(
    Printf.sprintf(
      "cursor index %d does not match %d",
      index,
      expected_index,
    ),
    index == expected_index,
  );
  assert_bool(
    Printf.sprintf("cursor row %d does not match %d", row, expected_row),
    row == expected_row,
  );
  assert_bool(
    Printf.sprintf(
      "cursor column %d does not match %d",
      column,
      expected_column,
    ),
    column == expected_column,
  );
};

let read_full_input = file => {
  let stream = _load(file);

  let rec next = s =>
    switch (FileStream.next(stream)) {
    | Some(ch) => next(s ++ String.make(1, ch))
    | None => s
    };
  let full_text = next("");

  assert_bool(
    "FileStream output string does not match",
    full_text == __raw_text,
  );
};

let read_and_check_cursor = (index, row, column) => {
  let stream = _load(__unix_file);

  let rec cursor_from = count =>
    if (count > 0) {
      FileStream.junk(stream);
      cursor_from(count - 1);
    } else {
      FileStream.cursor(stream);
    };

  _assert_cursor(cursor_from(index), index, row, column);

  FileStream.close(stream);
};

let () =
  run_test_tt_main(
    "FileStrem"
    >::: [
      "read file with unix endlines"
      >:: (test_ctx => read_full_input(__unix_file)),
      "read file with windows endlines"
      >:: (_ => read_full_input(__windows_file)),
      "read file and check cursor"
      >:: (
        _ => {
          read_and_check_cursor(3, 1, 3);
          read_and_check_cursor(18, 2, 2);
        }
      ),
    ],
  );