open Globals;
open MemberType;

let (|~>) = ((x, y), f) =>
  f(x)
  |> (
    switch (y^) {
    | None => Printf.sprintf("Unanalyzed(%s)")
    | Some(res) =>
      switch (res^) {
      | Inferred(_) => Printf.sprintf("Inferred(%s)")
      | Declared(_) => Printf.sprintf("Declared(%s)")
      }
    }
  );
