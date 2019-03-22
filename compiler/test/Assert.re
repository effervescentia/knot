open OUnit2;

let assert_string_eql = (actual, expected) =>
  assert_equal(
    ~msg="string match",
    ~printer=String.escaped,
    expected,
    actual,
  );

let assert_tkn_eql = (actual, expected) =>
  assert_equal(
    ~msg="token match",
    ~printer=KnotLex.Debug.print_tkn,
    expected,
    actual,
  );

let assert_ast_eql = (actual, expected) =>
  assert_equal(
    ~msg="AST match",
    ~printer=KnotParse.Debug.print_ast,
    expected,
    actual,
  );

let assert_stmt_eql = (actual, expected) =>
  assert_equal(
    ~msg="statement match",
    ~printer=KnotParse.Debug.print_stmt,
    expected,
    actual,
  );

let assert_decl_eql = (actual, expected) => {
  assert_equal(~msg="declaration name match", fst(expected), fst(actual));
  assert_equal(
    ~msg="declaration match",
    ~printer=KnotParse.Debug.print_decl(fst(actual)),
    snd(expected),
    snd(actual),
  );
};

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
