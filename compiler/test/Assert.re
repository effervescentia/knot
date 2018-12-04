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