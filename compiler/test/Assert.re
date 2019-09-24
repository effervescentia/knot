open OUnit2;
open Knot.Globals;

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

let assert_annotated_ast_eql = (actual, expected) =>
  assert_equal(
    ~msg="AST match",
    ~printer=fst % KnotParse.Debug.print_ast,
    ~cmp=Compare.compare_modules,
    expected,
    actual,
  );

let assert_import_eql = (actual, expected) =>
  assert_equal(
    ~msg="import match",
    ~printer=KnotParse.Debug.print_module_import,
    expected,
    actual,
  );

let assert_stmt_eql = (actual, expected) =>
  assert_equal(
    ~msg="statement match",
    ~printer=KnotParse.Debug.print_module_stmt,
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
        Printf.sprintf(
          "'%s' at [%d:%d]",
          Knot.Util.print_uchar(ch),
          row,
          col,
        ),
    ~cmp=
      (lhs, rhs) =>
        Uchar.equal(fst(lhs), fst(rhs))
        && fst(snd(lhs)) == fst(snd(rhs))
        && snd(snd(lhs)) == snd(snd(rhs)),
    expected,
    actual,
  );
