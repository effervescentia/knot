open OUnit2;

let assert_string_eql = (actual, expected) =>
  assert_equal(
    ~msg="string match",
    ~printer=String.escaped,
    expected,
    actual,
  );

/* let assert_char_eql = (actual, expected) =>
   assert_bool(
     Printf.sprintf(
       "expected '%c' (%d) to equal '%c'",
       actual,
       int_of_char(actual),
       expected,
     )
     |> String.escaped,
     expected == actual,
   ); */

/* let assert_int_eql = (actual, expected) =>
   assert_bool(
     Printf.sprintf("expected %d to equal %d", actual, expected),
     expected == actual,
   ); */

let assert_tkn_eql = (actual, expected) =>
  assert_equal(
    ~msg="token match",
    ~printer=KnotLex.Debug.print_tkn,
    expected,
    actual,
  );

let assert_ast_eql = (actual, expected) =>
  assert_equal(
    ~msg="ast match",
    ~printer=KnotParse.Debug.print_ast,
    expected,
    actual,
  );

let assert_cursor_eql = (actual, expected) =>
  assert_equal(
    ~msg="file cursor match",
    ~printer=
      ((ch, (row, col))) =>
        Printf.sprintf("'%c' at [%d:%d]", ch, row, col),
    ~cmp=
      (lhs, rhs) =>
        fst(lhs) == fst(rhs)
        && fst(snd(lhs)) == fst(snd(rhs))
        && snd(snd(lhs)) == snd(snd(rhs)),
    expected,
    actual,
  );