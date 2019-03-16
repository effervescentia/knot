open Globals;
open MemberType;

let (|~>) = ((x, y), f) =>
  f(x)
  |> (
    switch (y^ ^) {
    | Unanalyzed => Printf.sprintf("Unanalyzed(%s)")
    | Inferred(_) => Printf.sprintf("Inferred(%s)")
    | Declared(_) => Printf.sprintf("Declared(%s)")
    }
  );
