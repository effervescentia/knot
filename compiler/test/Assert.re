open OUnit2;

let assert_string_eql = (actual, expected) =>
  assert_bool(
    Printf.sprintf("expected \"%s\" to equal \"%s\"", actual, expected),
    expected == actual,
  );

let assert_char_eql = (actual, expected) =>
  assert_bool(
    Printf.sprintf(
      "expected '%c' (%d) to equal '%c'",
      actual,
      int_of_char(actual),
      expected,
    )
    |> String.escaped,
    expected == actual,
  );

let assert_int_eql = (actual, expected) =>
  assert_bool(
    Printf.sprintf("expected %d to equal %d", actual, expected),
    expected == actual,
  );

let assert_cursor =
    ((actual_ch, actual_cursor), (expected_ch, expected_cursor)) =>
  assert_bool(
    Printf.sprintf(
      "expected '%c' at [%d:%d] to match '%c' at [%d:%d]",
      actual_ch,
      fst(actual_cursor),
      snd(actual_cursor),
      expected_ch,
      fst(expected_cursor),
      snd(expected_cursor),
    ),
    actual_ch == expected_ch
    && fst(actual_cursor) == fst(expected_cursor)
    && snd(actual_cursor) == snd(expected_cursor),
  );