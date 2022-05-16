module type Target = {
  type t;

  let test: (t, t) => unit;
};

let throws = (exn, msg, test) =>
  Alcotest.check_raises(msg, exn, () => test() |> ignore);

module Make = (T: Target) => {
  include T;
};

module Compare = {
  let uchar =
    Alcotest.testable(
      (ppf, uchar) => {
        let buffer = Buffer.create(1);
        Buffer.add_utf_8_uchar(buffer, uchar);

        Buffer.contents(buffer) |> Format.pp_print_string(ppf);
      },
      (==),
    );

  let hashtbl = (key_to_string, value_to_string) =>
    Alcotest.testable(
      (ppf, tbl) =>
        tbl
        |> Hashtbl.to_seq_keys
        |> List.of_seq
        |> List.map(key =>
             Printf.sprintf(
               "%s: %s\n",
               key |> key_to_string,
               Hashtbl.find(tbl, key) |> value_to_string,
             )
           )
        |> List.fold_left((++), "")
        |> Printf.sprintf("{\n%s}")
        |> Format.pp_print_string(ppf),
      (l, r) =>
        Hashtbl.length(l) == Hashtbl.length(r)
        && Hashtbl.to_seq_keys(l)
        |> List.of_seq
        |> List.for_all(key =>
             Hashtbl.mem(r, key)
             && Hashtbl.find(l, key) == Hashtbl.find(r, key)
           ),
    );
};

let fail = Alcotest.fail;

let bool = Alcotest.(check(bool, "boolean matches"));
let true_ = Alcotest.(check(bool, "is true", true));
let false_ = Alcotest.(check(bool, "is false", false));
let string = Alcotest.(check(string, "string matches"));
let int = Alcotest.(check(int, "int matches"));

let uchar = Alcotest.(check(Compare.uchar, "uchar matches"));

let opt_int = Alcotest.(check(option(int), "int matches"));
let opt_string = Alcotest.(check(option(string), "string matches"));

let string_pair =
  Alcotest.(check(pair(string, string), "string pair matches"));
let int_pair = Alcotest.(check(pair(int, int), "int pair matches"));

let int_trio = Alcotest.(check(pair(int, int), "int trio matches"));

let char_list = Alcotest.(check(list(char), "char list matches"));
let string_list = Alcotest.(check(list(string), "string list matches"));
let int_list = Alcotest.(check(list(int), "int list matches"));

let string_pair_list =
  Alcotest.(check(list(pair(string, string)), "string pair list matches"));
let int_pair_list =
  Alcotest.(check(list(pair(int, int)), "int pair list matches"));

let pair_int_list =
  Alcotest.(check(pair(list(int), list(int)), "pair int list matches"));

let hashtbl = (key_to_string, value_to_string) =>
  Alcotest.(
    check(Compare.hashtbl(key_to_string, value_to_string), "hashtbl matches")
  );
