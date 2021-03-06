open Kore;

include Test.Assert;

let cursor =
  Alcotest.(
    check(
      testable(pp => Cursor.to_string % Format.pp_print_string(pp), (==)),
      "cursor matches",
    )
  );
