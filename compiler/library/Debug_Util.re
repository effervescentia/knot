open Globals;
open AnalysisContext;

let (|~>) = (x, f) =>
  abandon_ctx(x)
  |> f
  % (
    switch (x^) {
    | Unanalyzed(_) => Printf.sprintf("unanalyzed(%s)")
    | Pending(_, ls) =>
      Printf.sprintf("Promise.pending(%d, %s)", List.length(ls))
    | Resolved(_, _) => Printf.sprintf("Promise.resolved(%s)")
    }
  );
