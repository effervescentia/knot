open Kore;

let is_main: A.export_t => bool =
  fun
  | MainExport(_) => true
  | NamedExport(_) => false;

let create_scope = (range: Range.t, ctx: ModuleContext.t) =>
  ctx |> ModuleContext.to_scope(range);
