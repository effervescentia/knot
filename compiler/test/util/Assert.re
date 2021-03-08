module type Target = {
  type t;

  let test: (t, t) => unit;
};

let test_many = test => List.iter(((exp, act)) => test(exp, act));
let test_all = (test, exp) => List.iter(act => test(exp, act));

module Make = (T: Target) => {
  include T;

  let test_many = List.iter(((i, o)) => test(o, i));

  let test_all = o => List.iter(i => test(o, i));
};

let fail = Alcotest.fail;

let bool = Alcotest.(check(bool, "boolean matches"));
let true_ = Alcotest.(check(bool, "is true", true));
let false_ = Alcotest.(check(bool, "is false", false));
let string = Alcotest.(check(string, "string matches"));
let int = Alcotest.(check(int, "int matches"));
let uchar =
  Alcotest.(
    check(
      testable(
        (pp, uchar) => {
          let buffer = Buffer.create(1);
          Buffer.add_utf_8_uchar(buffer, uchar);

          Buffer.contents(buffer) |> Format.pp_print_string(pp);
        },
        (==),
      ),
      "uchar matches",
    )
  );

let opt_int = Alcotest.(check(option(int), "int matches"));

let int_pair = Alcotest.(check(pair(int, int), "int pair matches"));

let string_list = Alcotest.(check(list(string), "string list matches"));
let int_list = Alcotest.(check(list(int), "int list matches"));

let string_pair_list =
  Alcotest.(check(list(pair(string, string)), "string pair list matches"));
let int_pair_list =
  Alcotest.(check(list(pair(int, int)), "int pair list matches"));
