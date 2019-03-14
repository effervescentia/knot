open Globals;
open AnalysisContext;

let (|~>) = ((x, y), f) =>
  f(x)
  |> (
    switch (y^ ^) {
    | Unanalyzed => Printf.sprintf("Unanalyzed(%s)")
    | Pending(ls) => Printf.sprintf("Pending(%d, %s)", List.length(ls))
    | Synthetic(_, _) => Printf.sprintf("Synthetic(%s)")
    | Resolved(_) => Printf.sprintf("Resolved(%s)")
    }
  );
