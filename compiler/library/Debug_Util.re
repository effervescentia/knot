open Globals;
open AnalysisContext;

let (|~>) = ((x, y), f) =>
  f(x)
  |> (
    switch (y^) {
    | Unanalyzed => Printf.sprintf("unanalyzed(%s)")
    | Pending(ls) =>
      Printf.sprintf("Promise.pending(%d, %s)", List.length(ls))
    | Resolved(_) => Printf.sprintf("Promise.resolved(%s)")
    }
  );
